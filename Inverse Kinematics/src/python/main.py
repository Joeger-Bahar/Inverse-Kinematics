from renderer import Renderer
from arm import Arm
import pygame
import time
import serial

def normalize_servo_angle(angle):
    wrapped = angle % 360.0
    if wrapped > 180.0:
        wrapped = 360.0 - wrapped
    return max(0.0, min(180.0, wrapped))


def main():
    renderer = Renderer("Inverse Kinematics", 1200, 700)
    arm = Arm(renderer.sideBase[0], renderer.sideBase[1], 3, [85, 60, 40], 10)

    ser = serial.Serial('COM8', 115200)
    time.sleep(2.0)  # Allow Arduino reset/boot after serial open

    while Renderer.running:
        seg_count = renderer.update()
        if seg_count > 0:
            arm = Arm(
                renderer.sideBase[0],
                renderer.sideBase[1],
                seg_count,
                [700 // seg_count] * seg_count,
                10
            )

        target_x, target_y = renderer.get_committed_side_target()
        arm.update(target_x, target_y, renderer.ikSpeedScalar)
        arm.render(renderer.screen)

        renderer.render()
        time.sleep(0.05)  # ~20 FPS
        # Send angles to Arduino
        # Make angles relative to parent
        angles = []
        for seg in arm.segments:
            abs_angle = seg.angle + 180.0
            parent_angle = 0.0
            if seg.parent:
                parent_angle = seg.parent.angle + 90.0
            angle = normalize_servo_angle(abs_angle - parent_angle)
            angles.append(float(angle))

        angle_str = ','.join(map(str, angles))
        angle_str += ",0.0\n"
        yaw = renderer.get_committed_yaw()
        angle_str = f"{yaw:.1f}," + angle_str
        #print(angle_str)
        ser.write(angle_str.encode('utf-8'))

    pygame.quit()

if __name__ == "__main__":
    main()
