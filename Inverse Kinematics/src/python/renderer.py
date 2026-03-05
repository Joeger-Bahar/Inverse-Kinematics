import math
import pygame


class Renderer:
    running = True
    render_textures = True

    mouseX = mouseY = 0
    prevMouseX = prevMouseY = 0
    screenWidth = screenHeight = 0

    loaded_textures = []
    loaded_texture_rects = []

    def __init__(self, title, width, height, floor_offset_px=20.0, slot_count=5):
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption(title)
        Renderer.screenWidth = width
        Renderer.screenHeight = height

        self.clock = pygame.time.Clock()
        self.runningSegCount = 0
        self.ikSpeedScalar = 10.0
        self.coordInputMode = False
        self.coordInputBuffer = ""

        self.font = pygame.font.Font(None, 24)
        self.smallFont = pygame.font.Font(None, 18)

        half_width = width // 2
        self.topViewRect = pygame.Rect(0, 0, half_width, height)
        self.sideViewRect = pygame.Rect(half_width, 0, width - half_width, height)

        self.topBase = (self.topViewRect.centerx, self.topViewRect.centery)
        self.sideBase = (self.sideViewRect.centerx, self.sideViewRect.centery)
        self.floorOffsetPx = float(floor_offset_px)
        self.floorY = float(self.sideBase[1] + self.floorOffsetPx)

        self.maxRadius = min(
            self.sideViewRect.width - 30,
            (self.topViewRect.width // 2) - 30
        )

        self.selectionMode = True
        default_target = {
            "radius": 140.0,
            "y": float(self.sideBase[1]),
            "yaw": 90.0
        }
        self.pendingTarget = self._normalize_target(default_target)
        self.committedTarget = dict(self.pendingTarget)
        self.currentArmRadius = self.pendingTarget["radius"]

        self.slotCount = max(1, int(slot_count))
        self.savedSlots = [None] * self.slotCount
        self.pendingSlotSaves = []

        self.actualEndEffectorSide = (
            float(self.sideBase[0] + self.committedTarget["radius"]),
            float(self.committedTarget["y"])
        )

    @staticmethod
    def load_texture(path):
        surf = pygame.image.load(path).convert_alpha()
        Renderer.loaded_textures.append(surf)
        Renderer.loaded_texture_rects.append(surf.get_rect())
        return len(Renderer.loaded_textures) - 1

    def _clamp(self, value, low, high):
        return max(low, min(high, value))

    def _side_y_bounds(self):
        low = float(self.sideViewRect.top + 10)
        high = float(min(self.floorY, self.sideViewRect.bottom - 10))
        if high < low:
            high = low
        return low, high

    def _clamp_side_y(self, value):
        low, high = self._side_y_bounds()
        return float(self._clamp(value, low, high))

    def _normalize_target(self, target):
        return {
            "radius": float(self._clamp(target.get("radius", 0.0), 0.0, self.maxRadius)),
            "y": self._clamp_side_y(target.get("y", self.sideBase[1])),
            "yaw": float(self._clamp(target.get("yaw", 0.0), 0.0, 180.0))
        }

    def _extract_side_target_from_pos(self, pos):
        if not self.sideViewRect.collidepoint(pos):
            return None

        x_pos, y_pos = pos
        return {
            "radius": float(self._clamp(x_pos - self.sideBase[0], 0.0, self.maxRadius)),
            "y": self._clamp_side_y(y_pos)
        }

    def _update_pending_from_mouse(self, pos):
        side_target = self._extract_side_target_from_pos(pos)
        if side_target is None:
            return
        self.pendingTarget["radius"] = side_target["radius"]
        self.pendingTarget["y"] = side_target["y"]

    def _update_follow_from_mouse(self, pos):
        side_target = self._extract_side_target_from_pos(pos)
        if side_target is None:
            return

        self.committedTarget["radius"] = side_target["radius"]
        self.committedTarget["y"] = side_target["y"]
        self.pendingTarget["radius"] = side_target["radius"]
        self.pendingTarget["y"] = side_target["y"]

    def _xyz_to_target(self, x_val, y_val, z_val, yaw_fallback=None):
        x_val = self._clamp(x_val, self.topViewRect.left, self.topViewRect.right - 1)
        y_val = self._clamp_side_y(y_val)
        z_val = self._clamp(z_val, self.topViewRect.top, self.topViewRect.bottom - 1)

        dx = x_val - self.topBase[0]
        dz_screen = z_val - self.topBase[1]
        radius = self._clamp(math.hypot(dx, dz_screen), 0.0, self.maxRadius)

        if radius <= 1e-6:
            if yaw_fallback is None:
                yaw = self.pendingTarget["yaw"]
            else:
                yaw = float(yaw_fallback)
        else:
            yaw = math.degrees(math.atan2(-dz_screen, dx))
            if yaw < 0.0:
                yaw += 360.0
        yaw = self._clamp(yaw, 0.0, 180.0)

        return {
            "radius": float(radius),
            "y": float(y_val),
            "yaw": float(yaw)
        }

    def _target_to_xyz(self, target):
        yaw_rad = math.radians(target["yaw"])
        radius = target["radius"]
        x_pos = self.topBase[0] + radius * math.cos(yaw_rad)
        z_pos = self.topBase[1] - radius * math.sin(yaw_rad)
        return (float(x_pos), float(target["y"]), float(z_pos))

    def _parse_3d_coord_input(self):
        cleaned = self.coordInputBuffer.replace(" ", "")
        parts = cleaned.split(",")
        if len(parts) != 3:
            return None

        try:
            x_val = float(parts[0])
            y_val = float(parts[1])
            z_val = float(parts[2])
        except ValueError:
            return None

        return self._xyz_to_target(
            x_val,
            y_val,
            z_val,
            yaw_fallback=self.pendingTarget["yaw"]
        )

    def _digit_to_slot_index(self, key):
        if pygame.K_1 <= key <= pygame.K_9:
            index = key - pygame.K_1
            if index < self.slotCount:
                return index
        return None

    def _queue_slot_save(self, slot_index):
        if slot_index < 0 or slot_index >= self.slotCount:
            return
        if slot_index not in self.pendingSlotSaves:
            self.pendingSlotSaves.append(slot_index)

    def _save_slot_from_actual(self, slot_index):
        if slot_index < 0 or slot_index >= self.slotCount:
            return

        side_x, side_y = self.actualEndEffectorSide
        radius = self._clamp(side_x - self.sideBase[0], 0.0, self.maxRadius)
        target = {
            "radius": float(radius),
            "y": self._clamp_side_y(side_y),
            "yaw": float(self.committedTarget["yaw"])
        }
        x_pos, y_pos, z_pos = self._target_to_xyz(target)
        self.savedSlots[slot_index] = {
            "x": float(x_pos),
            "y": float(y_pos),
            "z": float(z_pos)
        }

    def _recall_slot(self, slot_index):
        if slot_index < 0 or slot_index >= self.slotCount:
            return

        slot_data = self.savedSlots[slot_index]
        if slot_data is None:
            return

        target = self._xyz_to_target(
            slot_data["x"],
            slot_data["y"],
            slot_data["z"],
            yaw_fallback=self.committedTarget["yaw"]
        )
        self.pendingTarget = dict(target)
        self.committedTarget = dict(target)
        self.selectionMode = True

    def _draw_side_grid(self, spacing=50):
        minor_color = (28, 28, 28)
        major_color = (45, 45, 45)
        center_color = (70, 70, 70)
        label_color = (120, 120, 120)
        major_step = spacing * 2

        for x_pos in range(self.sideViewRect.left, self.sideViewRect.right + 1, spacing):
            rel = x_pos - self.sideViewRect.left
            is_major = (rel % major_step) == 0
            color = major_color if is_major else minor_color
            width = 2 if is_major else 1
            pygame.draw.line(
                self.screen,
                color,
                (x_pos, self.sideViewRect.top),
                (x_pos, self.sideViewRect.bottom),
                width
            )

        for y_pos in range(self.sideViewRect.top, self.sideViewRect.bottom + 1, spacing):
            rel = y_pos - self.sideViewRect.top
            is_major = (rel % major_step) == 0
            color = major_color if is_major else minor_color
            width = 2 if is_major else 1
            pygame.draw.line(
                self.screen,
                color,
                (self.sideViewRect.left, y_pos),
                (self.sideViewRect.right, y_pos),
                width
            )

        for y_pos in range(self.sideViewRect.top, self.sideViewRect.bottom + 1, major_step):
            label = self.smallFont.render(str(y_pos), True, label_color)
            self.screen.blit(label, (self.sideViewRect.left + 3, y_pos + 3))

        pygame.draw.line(
            self.screen,
            center_color,
            (self.sideBase[0], self.sideViewRect.top),
            (self.sideBase[0], self.sideViewRect.bottom),
            2
        )
        pygame.draw.line(
            self.screen,
            center_color,
            (self.sideViewRect.left, self.sideBase[1]),
            (self.sideViewRect.right, self.sideBase[1]),
            2
        )

    def _draw_top_grid(self, spacing=50):
        minor_color = (26, 26, 32)
        major_color = (40, 40, 50)
        center_color = (70, 70, 84)
        label_color = (110, 110, 128)
        major_step = spacing * 2

        for x_pos in range(self.topViewRect.left, self.topViewRect.right + 1, spacing):
            rel = x_pos - self.topViewRect.left
            is_major = (rel % major_step) == 0
            color = major_color if is_major else minor_color
            width = 2 if is_major else 1
            pygame.draw.line(
                self.screen,
                color,
                (x_pos, self.topViewRect.top),
                (x_pos, self.topViewRect.bottom),
                width
            )

        for z_pos in range(self.topViewRect.top, self.topViewRect.bottom + 1, spacing):
            rel = z_pos - self.topViewRect.top
            is_major = (rel % major_step) == 0
            color = major_color if is_major else minor_color
            width = 2 if is_major else 1
            pygame.draw.line(
                self.screen,
                color,
                (self.topViewRect.left, z_pos),
                (self.topViewRect.right, z_pos),
                width
            )

        for x_pos in range(self.topViewRect.left, self.topViewRect.right + 1, major_step):
            label = self.smallFont.render(str(x_pos), True, label_color)
            self.screen.blit(label, (x_pos + 3, self.topViewRect.top + 3))

        for z_pos in range(self.topViewRect.top, self.topViewRect.bottom + 1, major_step):
            label = self.smallFont.render(str(z_pos), True, label_color)
            self.screen.blit(label, (self.topViewRect.left + 3, z_pos + 3))

        pygame.draw.line(
            self.screen,
            center_color,
            (self.topBase[0], self.topViewRect.top),
            (self.topBase[0], self.topViewRect.bottom),
            2
        )
        pygame.draw.line(
            self.screen,
            center_color,
            (self.topViewRect.left, self.topBase[1]),
            (self.topViewRect.right, self.topBase[1]),
            2
        )

    def _draw_top_view(self):
        panel_color = (13, 13, 16)
        self.screen.fill(panel_color, self.topViewRect)
        self._draw_top_grid()

        pygame.draw.circle(self.screen, (90, 90, 100), self.topBase, 4)

    def _draw_side_view_panel(self):
        panel_color = (9, 9, 9)
        self.screen.fill(panel_color, self.sideViewRect)
        self._draw_side_grid()

        floor_color = (180, 120, 60)
        floor_y = int(self.floorY)
        pygame.draw.line(
            self.screen,
            floor_color,
            (self.sideViewRect.left, floor_y),
            (self.sideViewRect.right, floor_y),
            2
        )
        floor_label = self.smallFont.render("Floor", True, floor_color)
        self.screen.blit(floor_label, (self.sideViewRect.right - 48, floor_y - 18))

    def _target_top_point(self, target):
        yaw_rad = math.radians(target["yaw"])
        radius = target["radius"]
        x_pos = self.topBase[0] + radius * math.cos(yaw_rad)
        z_pos = self.topBase[1] - radius * math.sin(yaw_rad)
        return (int(x_pos), int(z_pos))

    def _target_side_point(self, target):
        x_pos = self.sideBase[0] + target["radius"]
        y_pos = target["y"]
        return (int(x_pos), int(y_pos))

    def _draw_target_markers(self):
        committed_side = self._target_side_point(self.committedTarget)
        committed_top = self._target_top_point(self.committedTarget)

        pygame.draw.circle(self.screen, (230, 40, 40), committed_side, 8, 2)
        pygame.draw.circle(self.screen, (230, 40, 40), committed_side, 2)
        pygame.draw.circle(self.screen, (230, 40, 40), committed_top, 8, 2)
        pygame.draw.circle(self.screen, (230, 40, 40), committed_top, 2)

        if self.selectionMode:
            pending_side = self._target_side_point(self.pendingTarget)
            pending_top = self._target_top_point(self.pendingTarget)
            pygame.draw.circle(self.screen, (240, 190, 60), pending_side, 7, 2)
            pygame.draw.circle(self.screen, (240, 190, 60), pending_top, 7, 2)

        committed_label = self.smallFont.render(
            f"Committed yaw: {self.committedTarget['yaw']:.1f}",
            True,
            (230, 90, 90)
        )
        self.screen.blit(committed_label, (self.topViewRect.left + 10, self.topViewRect.bottom - 24))

    def _draw_top_arm(self):
        yaw_rad = math.radians(self.committedTarget["yaw"])
        x_tip = self.topBase[0] + self.currentArmRadius * math.cos(yaw_rad)
        z_tip = self.topBase[1] - self.currentArmRadius * math.sin(yaw_rad)
        tip = (int(x_tip), int(z_tip))

        pygame.draw.line(self.screen, (110, 220, 255), self.topBase, tip, 6)
        pygame.draw.circle(self.screen, (110, 220, 255), tip, 6)

    def _slot_state_text(self):
        parts = []
        for index, slot in enumerate(self.savedSlots):
            state = "SET" if slot is not None else "EMPTY"
            parts.append(f"{index + 1}:{state}")
        return " ".join(parts)

    def _draw_hud(self):
        mode_text = "SELECT" if self.selectionMode else "FOLLOW"
        committed_x, committed_y, committed_z = self._target_to_xyz(self.committedTarget)
        committed_xyz_text = f"{int(committed_x)},{int(committed_y)},{int(committed_z)}"
        coord_mode_text = "ON" if self.coordInputMode else "OFF"

        line1 = ""
        line2 = "[C] xyz input [Tab] mode [[/]] speed [1-5] recall [Shift+1-5] save"
        line3 = (
            f"mode:{mode_text} speed:{self.ikSpeedScalar:.1f}x "
            f"pending_yaw:{self.pendingTarget['yaw']:.1f} floor_y:{int(self.floorY)} "
            f"xyz_input:{coord_mode_text} committed_xyz:{committed_xyz_text} slots:{self._slot_state_text()}"
        )

        self.screen.blit(self.font.render(line1, True, (185, 185, 185)), (10, 10))
        self.screen.blit(self.font.render(line2, True, (185, 185, 185)), (10, 30))
        self.screen.blit(self.smallFont.render(line3, True, (180, 180, 180)), (10, 52))

        if self.coordInputMode:
            input_text = self.font.render(
                f"Enter XYZ as x,y,z: {self.coordInputBuffer}",
                True,
                (240, 210, 120)
            )
            self.screen.blit(input_text, (10, 74))

    def update(self):
        return_seg_count = 0

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                Renderer.running = False

            elif event.type == pygame.KEYDOWN:
                ctrl_held = (event.mod & pygame.KMOD_CTRL) != 0
                shift_held = (event.mod & pygame.KMOD_SHIFT) != 0

                if event.key == pygame.K_ESCAPE:
                    Renderer.running = False

                elif self.coordInputMode:
                    if event.key == pygame.K_RETURN:
                        parsed = self._parse_3d_coord_input()
                        if parsed is not None:
                            self.pendingTarget = dict(parsed)
                            self.committedTarget = dict(parsed)
                        self.coordInputMode = False
                        self.coordInputBuffer = ""

                    elif event.key == pygame.K_BACKSPACE:
                        self.coordInputBuffer = self.coordInputBuffer[:-1]

                    elif event.unicode and event.unicode in "0123456789,.- ":
                        self.coordInputBuffer += event.unicode

                elif event.key == pygame.K_c:
                    self.coordInputMode = True
                    self.coordInputBuffer = ""

                elif event.key == pygame.K_TAB:
                    self.selectionMode = not self.selectionMode
                    if not self.selectionMode:
                        self._update_follow_from_mouse((Renderer.mouseX, Renderer.mouseY))

                elif event.key == pygame.K_RETURN:
                    if ctrl_held:
                        return_seg_count = self.runningSegCount
                        self.runningSegCount = 0
                    elif self.selectionMode:
                        self.committedTarget = dict(self.pendingTarget)

                elif event.key == pygame.K_BACKSPACE and ctrl_held:
                    self.runningSegCount //= 10

                elif ctrl_held and pygame.K_0 <= event.key <= pygame.K_9:
                    self.runningSegCount *= 10
                    self.runningSegCount += event.key - pygame.K_0

                elif pygame.K_1 <= event.key <= pygame.K_9:
                    slot_index = self._digit_to_slot_index(event.key)
                    if slot_index is not None:
                        if shift_held:
                            self._queue_slot_save(slot_index)
                        else:
                            self._recall_slot(slot_index)

                elif event.key == pygame.K_LEFTBRACKET:
                    self.ikSpeedScalar = max(0.1, self.ikSpeedScalar - 1.0)

                elif event.key == pygame.K_RIGHTBRACKET:
                    self.ikSpeedScalar = min(200.0, self.ikSpeedScalar + 1.0)

                elif event.key == pygame.K_a:
                    Renderer.render_textures = not Renderer.render_textures

            elif event.type == pygame.MOUSEMOTION:
                Renderer.prevMouseX = Renderer.mouseX
                Renderer.prevMouseY = Renderer.mouseY
                Renderer.mouseX, Renderer.mouseY = event.pos

                if self.coordInputMode:
                    continue
                if self.selectionMode:
                    self._update_pending_from_mouse(event.pos)
                else:
                    self._update_follow_from_mouse(event.pos)

            elif event.type == pygame.MOUSEBUTTONDOWN:
                if self.coordInputMode:
                    continue
                if self.selectionMode:
                    self._update_pending_from_mouse(event.pos)
                else:
                    self._update_follow_from_mouse(event.pos)

        if self.selectionMode and not self.coordInputMode:
            keys = pygame.key.get_pressed()
            yaw_delta = 0.0
            if keys[pygame.K_LEFT]:
                yaw_delta += 1.5
            if keys[pygame.K_RIGHT]:
                yaw_delta -= 1.5
            if yaw_delta != 0.0:
                self.pendingTarget["yaw"] = self._clamp(
                    self.pendingTarget["yaw"] + yaw_delta,
                    0.0,
                    180.0
                )

        return return_seg_count

    def clear(self):
        self.screen.fill((9, 9, 9))
        self._draw_top_view()
        self._draw_side_view_panel()
        pygame.draw.line(
            self.screen,
            (90, 90, 90),
            (self.sideViewRect.left, 0),
            (self.sideViewRect.left, Renderer.screenHeight),
            2
        )
        self._draw_top_arm()
        self._draw_target_markers()
        self._draw_hud()

    def render(self):
        pygame.display.flip()
        self.clock.tick(60)
        self.clear()

    def get_committed_side_target(self):
        return (
            float(self.sideBase[0] + self.committedTarget["radius"]),
            float(self.committedTarget["y"])
        )

    def get_committed_yaw(self):
        return float(self.committedTarget["yaw"])

    def set_arm_projection_radius(self, radius):
        self.currentArmRadius = float(self._clamp(radius, 0.0, self.maxRadius))

    def set_actual_end_effector_side(self, x, y):
        min_x = float(self.sideBase[0])
        max_x = float(self.sideBase[0] + self.maxRadius)
        clamped_x = float(self._clamp(x, min_x, max_x))
        clamped_y = self._clamp_side_y(y)
        self.actualEndEffectorSide = (clamped_x, clamped_y)

    def flush_pending_slot_saves(self):
        while self.pendingSlotSaves:
            slot_index = self.pendingSlotSaves.pop(0)
            self._save_slot_from_actual(slot_index)
