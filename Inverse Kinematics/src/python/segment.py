import math
import pygame
from pygame.math import Vector2
from renderer import Renderer


def lerp(a, b, t):
    return a + (b - a) * t


def normalize_signed_angle(angle_deg):
    return ((angle_deg + 180.0) % 360.0) - 180.0


class Segment:
    ELBOW_UP_MIN_REL = 0.0
    ELBOW_UP_MAX_REL = 135.0
    ELBOW_DOWN_MIN_REL = -135.0
    ELBOW_DOWN_MAX_REL = 0.0

    def __init__(self, length, width, angle, thickness,
                 texture_index=-1, color=(255, 255, 255, 255),
                 parent=None, child=None):

        self.length = length
        self.width = width
        self.angle = angle
        self.thickness = thickness
        self.texture_index = texture_index
        self.color = color

        self.parent = parent
        self.child = child

        self.a = Vector2()
        self.b = Vector2()
        self.relationDir = Vector2()

        self.interpolationSpeed = 1.0
        self.maxDistance = 0.0

        self.haveLast = False
        self.lastA = 0.0
        self.unwrapped = 0.0
        self.haveLastTarget = False
        self.lastTarget = Vector2()

    def _rebuild_relation_from_angle(self):
        self.relationDir = Vector2(
            math.cos(math.radians(self.angle)),
            math.sin(math.radians(self.angle))
        ) * self.length

    def _constrain_endpoint_to_floor(self, floor_y=None):
        if floor_y is None or self.b.y <= floor_y:
            return

        dy = floor_y - self.a.y
        dy = max(-self.length, min(self.length, dy))
        x_mag_sq = (self.length * self.length) - (dy * dy)
        x_mag = math.sqrt(max(0.0, x_mag_sq))

        x_component = self.relationDir.x
        if abs(x_component) < 1e-6:
            x_component = self.b.x - self.a.x
        x_sign = 1.0 if x_component >= 0.0 else -1.0

        self.relationDir = Vector2(x_sign * x_mag, dy)
        self.angle = math.degrees(math.atan2(self.relationDir.y, self.relationDir.x))
        self.b = self.a + self.relationDir

    def assign_child(self, child):
        self.child = child

    def assign_parent(self, parent):
        self.parent = parent

    def render(self, surface):
        pygame.draw.line(
            surface,
            self.color,
            self.a,
            self.b,
            self.thickness
        )

        if self.child:
            self.child.render(surface)

    def reverse_k(self, mouseX=-1, mouseY=-1, speed_scalar=1.0, elbow_up=True):
        if not self.child:
            current_target = Vector2(mouseX, mouseY)
            if self.haveLastTarget:
                dist = current_target.distance_to(self.lastTarget)
            else:
                dist = 0.0
                self.haveLastTarget = True
            self.lastTarget = current_target
            base_speed = min(1.0 / (1.0 + dist), 0.1)
            self.interpolationSpeed = max(0.0, min(base_speed * speed_scalar, 1.0))
            target = lerp(self.b, current_target, self.interpolationSpeed)
        else:
            target = self.child.a

        direction = target - self.a
        desiredAngle = math.degrees(math.atan2(direction.y, direction.x))

        if self.parent:
            parent_angle = math.degrees(
                math.atan2(self.parent.relationDir.y, self.parent.relationDir.x)
            )
            rel = normalize_signed_angle(desiredAngle - parent_angle)
            if elbow_up:
                rel = abs(rel)
                rel = max(self.ELBOW_UP_MIN_REL, min(rel, self.ELBOW_UP_MAX_REL))
            else:
                rel = -abs(rel)
                rel = max(self.ELBOW_DOWN_MIN_REL, min(rel, self.ELBOW_DOWN_MAX_REL))
            desiredAngle = parent_angle + rel

        if not self.haveLast:
            self.lastA = desiredAngle
            self.unwrapped = desiredAngle
            self.haveLast = True
        else:
            delta = desiredAngle - self.lastA
            if delta > 180:
                delta -= 360
            elif delta < -180:
                delta += 360
            self.unwrapped += delta
            self.lastA = desiredAngle

        # parent_angle = (
        #   math.degrees(math.atan2(self.parent.relationDir.y, self.parent.relationDir.x))
        #   if self.parent
        #   else -90.0
        # )

        # min_angle = -30.0 + parent_angle
        # max_angle = 150.0 + parent_angle
        # self.angle = max(min(self.unwrapped, max_angle), min_angle)
        self.angle = self.unwrapped

        self.b = target
        self._rebuild_relation_from_angle()
        self.a = self.b - self.relationDir

        if self.parent:
            self.parent.reverse_k(speed_scalar=speed_scalar, elbow_up=elbow_up)

    def forward_k(self, floor_y=None, elbow_up=True):
        if self.parent:
            parent_angle = math.degrees(
                math.atan2(self.parent.relationDir.y, self.parent.relationDir.x)
            )

            rel = normalize_signed_angle(self.angle - parent_angle)
            if elbow_up:
                rel = abs(rel)
                rel = max(self.ELBOW_UP_MIN_REL, min(rel, self.ELBOW_UP_MAX_REL))
            else:
                rel = -abs(rel)
                rel = max(self.ELBOW_DOWN_MIN_REL, min(rel, self.ELBOW_DOWN_MAX_REL))
            self.angle = parent_angle + rel

            self._rebuild_relation_from_angle()

            direction = self.a - self.parent.b
            dist = direction.length()

            if dist > 0 and self.maxDistance > 0:
                if dist > self.maxDistance:
                    direction.scale_to_length(self.maxDistance)
            else:
                direction = Vector2()

            self.a = self.parent.b + direction

        self.b = self.a + self.relationDir
        self._constrain_endpoint_to_floor(floor_y=floor_y)

        if self.child:
            self.child.forward_k(floor_y=floor_y, elbow_up=elbow_up)


class BaseSegment(Segment):
    def forward_k(self, floor_y=None, elbow_up=True):
        if hasattr(self, "base_position"):
            self.a = Vector2(self.base_position)
        else:
            self.a = Vector2(Renderer.screenWidth / 2, Renderer.screenHeight / 2)
        self.b = self.a + self.relationDir
        self._constrain_endpoint_to_floor(floor_y=floor_y)
        if self.child:
            self.child.forward_k(floor_y=floor_y, elbow_up=elbow_up)
