from segment import Segment, BaseSegment
from renderer import Renderer
import pygame

class Arm:
    def __init__(
        self,
        baseX,
        baseY,
        segCount,
        segLengths,
        segWidth,
        floor_offset_px=20.0,
        elbow_up=True
    ):
        self.baseX = baseX
        self.baseY = baseY
        self.segCount = segCount
        self.segWidth = segWidth
        self.floorOffsetPx = float(floor_offset_px)
        self.floorY = float(baseY + self.floorOffsetPx)
        self.elbowUp = bool(elbow_up)

        self.segments = []

        self.baseSeg = BaseSegment(segLengths[0], 50, 0.0, 10)
        self.baseSeg.base_position = (baseX, baseY)
        self.segments.append(self.baseSeg)

        for i in range(1, segCount):
            color = (255 - i * 25, i + 25, 255, 255)
            seg = Segment(
                segLengths[i],
                50,
                90.0,
                max(1, 10 - (i // ((segCount // 10) + 1))),
                color=color
            )
            seg.assign_parent(self.segments[i - 1])
            self.segments[i - 1].assign_child(seg)
            self.segments.append(seg)

    def update(self, target_x=None, target_y=None, speed_scalar=1.0):
        if target_x is None or target_y is None:
            target_x = Renderer.mouseX
            target_y = Renderer.mouseY
        target_y = min(float(target_y), self.floorY)
        self.segments[-1].reverse_k(
            target_x,
            target_y,
            speed_scalar=speed_scalar,
            elbow_up=self.elbowUp
        )
        self.baseSeg.forward_k(floor_y=self.floorY, elbow_up=self.elbowUp)

    def render(self, surface):
        self.baseSeg.render(surface)
        end_effector = self.segments[-1].b
        pygame.draw.circle(
            surface,
            (255, 80, 80),
            (int(end_effector.x), int(end_effector.y)),
            8,
            2
        )

    def get_end_effector_side(self):
        end_effector = self.segments[-1].b
        return (float(end_effector.x), float(end_effector.y))
