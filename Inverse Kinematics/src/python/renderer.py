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

    def __init__(self, title, width, height):
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption(title)
        Renderer.screenWidth = width
        Renderer.screenHeight = height

        self.clock = pygame.time.Clock()
        self.runningSegCount = 0
        self.ikSpeedScalar = 10.0

        self.font = pygame.font.Font(None, 24)
        self.smallFont = pygame.font.Font(None, 18)

        half_width = width // 2
        self.topViewRect = pygame.Rect(0, 0, half_width, height)
        self.sideViewRect = pygame.Rect(half_width, 0, width - half_width, height)

        self.topBase = (self.topViewRect.centerx, self.topViewRect.centery)
        self.sideBase = (self.sideViewRect.centerx, self.sideViewRect.centery)

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
        self.pendingTarget = dict(default_target)
        self.committedTarget = dict(default_target)
        self.currentArmRadius = default_target["radius"]

    @staticmethod
    def load_texture(path):
        surf = pygame.image.load(path).convert_alpha()
        Renderer.loaded_textures.append(surf)
        Renderer.loaded_texture_rects.append(surf.get_rect())
        return len(Renderer.loaded_textures) - 1

    def _clamp(self, value, low, high):
        return max(low, min(high, value))

    def _update_pending_from_mouse(self, pos):
        if not self.sideViewRect.collidepoint(pos):
            return

        x_pos, y_pos = pos
        radius = float(self._clamp(x_pos - self.sideBase[0], 0, self.maxRadius))
        y_val = float(self._clamp(y_pos, self.sideViewRect.top + 10, self.sideViewRect.bottom - 10))

        self.pendingTarget["radius"] = radius
        self.pendingTarget["y"] = y_val

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

        for x_pos in range(self.sideViewRect.left, self.sideViewRect.right + 1, major_step):
            label = self.smallFont.render(str(x_pos), True, label_color)
            self.screen.blit(label, (x_pos + 3, self.sideViewRect.top + 3))

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

    def _draw_top_view(self):
        panel_color = (13, 13, 16)
        axis_color = (60, 60, 70)
        self.screen.fill(panel_color, self.topViewRect)

        pygame.draw.line(
            self.screen,
            axis_color,
            (self.topBase[0], self.topViewRect.top + 20),
            (self.topBase[0], self.topViewRect.bottom - 20),
            2
        )
        pygame.draw.line(
            self.screen,
            axis_color,
            (self.topViewRect.left + 20, self.topBase[1]),
            (self.topViewRect.right - 20, self.topBase[1]),
            2
        )

        pygame.draw.circle(self.screen, (90, 90, 100), self.topBase, 4)
        label = self.smallFont.render("Top-Down (X/Z)", True, (190, 190, 210))
        self.screen.blit(label, (self.topViewRect.left + 10, self.topViewRect.top + 8))

    def _draw_side_view_panel(self):
        panel_color = (9, 9, 9)
        self.screen.fill(panel_color, self.sideViewRect)
        self._draw_side_grid()
        label = self.smallFont.render("Side View (R/Y)", True, (190, 190, 210))
        self.screen.blit(label, (self.sideViewRect.left + 10, self.sideViewRect.top + 8))

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

    def _draw_hud(self):
        mode_text = "SELECT" if self.selectionMode else "FOLLOW"
        hud = (
            f"[Tab] mode  [Enter] commit  [<- ->] yaw  [[/]] speed | "
            f"mode: {mode_text} | speed: {self.ikSpeedScalar:.1f}x | "
            f"pending yaw: {self.pendingTarget['yaw']:.1f}"
        )
        text_surface = self.font.render(hud, True, (185, 185, 185))
        self.screen.blit(text_surface, (10, 10))

    def update(self):
        return_seg_count = 0

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                Renderer.running = False

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    Renderer.running = False

                elif event.key == pygame.K_TAB:
                    self.selectionMode = not self.selectionMode

                elif event.key == pygame.K_RETURN:
                    if self.selectionMode:
                        self.committedTarget = dict(self.pendingTarget)
                    else:
                        return_seg_count = self.runningSegCount
                        self.runningSegCount = 0

                elif event.key == pygame.K_LEFT:
                    if self.selectionMode:
                        self.pendingTarget["yaw"] = self._clamp(
                            self.pendingTarget["yaw"] - 2.0,
                            0.0,
                            180.0
                        )

                elif event.key == pygame.K_RIGHT:
                    if self.selectionMode:
                        self.pendingTarget["yaw"] = self._clamp(
                            self.pendingTarget["yaw"] + 2.0,
                            0.0,
                            180.0
                        )

                elif event.key == pygame.K_LEFTBRACKET:
                    self.ikSpeedScalar = max(0.1, self.ikSpeedScalar - 1.0)

                elif event.key == pygame.K_RIGHTBRACKET:
                    self.ikSpeedScalar = min(200.0, self.ikSpeedScalar + 1.0)

                elif pygame.K_0 <= event.key <= pygame.K_9:
                    self.runningSegCount *= 10
                    self.runningSegCount += event.key - pygame.K_0

                elif event.key == pygame.K_BACKSPACE:
                    self.runningSegCount //= 10

                elif event.key == pygame.K_a:
                    Renderer.render_textures = not Renderer.render_textures

            elif event.type == pygame.MOUSEMOTION:
                Renderer.prevMouseX = Renderer.mouseX
                Renderer.prevMouseY = Renderer.mouseY
                Renderer.mouseX, Renderer.mouseY = event.pos
                if self.selectionMode:
                    self._update_pending_from_mouse(event.pos)

            elif event.type == pygame.MOUSEBUTTONDOWN and self.selectionMode:
                self._update_pending_from_mouse(event.pos)

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
        return self._target_side_point(self.committedTarget)

    def get_committed_yaw(self):
        return float(self.committedTarget["yaw"])

    def set_arm_projection_radius(self, radius):
        self.currentArmRadius = float(self._clamp(radius, 0.0, self.maxRadius))
