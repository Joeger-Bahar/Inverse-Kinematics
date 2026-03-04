import math
import pygame
from pygame.math import Vector2
from renderer import Renderer


def lerp(a, b, t):
    return a + (b - a) * t


class Segment:
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

    def reverse_k(self, mouseX=-1, mouseY=-1, speed_scalar=1.0):
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
        self.relationDir = Vector2(
            math.cos(math.radians(self.angle)),
            math.sin(math.radians(self.angle))
        ) * self.length
        self.a = self.b - self.relationDir

        if self.parent:
            self.parent.reverse_k(speed_scalar=speed_scalar)

    def forward_k(self):
        if self.parent:
            parent_angle = math.degrees(
                math.atan2(self.parent.relationDir.y, self.parent.relationDir.x)
            )

            min_angle = parent_angle - 30.0
            max_angle = parent_angle + 150.0

            self.angle = max(min(self.angle, max_angle), min_angle)

            self.relationDir = Vector2(
                math.cos(math.radians(self.angle)),
                math.sin(math.radians(self.angle))
            ) * self.length

            direction = self.a - self.parent.b
            dist = direction.length()

            if dist > 0 and self.maxDistance > 0:
                if dist > self.maxDistance:
                    direction.scale_to_length(self.maxDistance)
            else:
                direction = Vector2()

            self.a = self.parent.b + direction

        self.b = self.a + self.relationDir

        if self.child:
            self.child.forward_k()


class BaseSegment(Segment):
    def forward_k(self):
        if hasattr(self, "base_position"):
            self.a = Vector2(self.base_position)
        else:
            self.a = Vector2(Renderer.screenWidth / 2, Renderer.screenHeight / 2)
        self.b = self.a + self.relationDir
        if self.child:
            self.child.forward_k()
