import math
import pygame

pygame.init()

WIDTH, HEIGHT = 600, 600
BG_COLOR = (220, 220, 220)
BLACK = (0, 0, 0)
YELLOW = (250, 245, 0)
GREEN = (0, 255, 0)
BLUE = (20, 20, 245)
RED = (255, 20, 20)

win = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Zadanie 2")


def square_surface(size, color):
    surf = pygame.Surface((size, size), pygame.SRCALPHA)
    pygame.draw.rect(surf, color, (0, 0, size, size))
    return surf


def circle_surface(size, color):
    surf = pygame.Surface((size, size), pygame.SRCALPHA)
    pygame.draw.circle(surf, color, (size // 2, size // 2), size // 2)
    return surf


def triangle_surface(size, color):
    surf = pygame.Surface((size, size), pygame.SRCALPHA)
    points = [(size // 2, 0), (0, size), (size, size)]
    pygame.draw.polygon(surf, color, points)
    return surf


base_square = square_surface(100, BLUE)
base_triangle = triangle_surface(100, BLUE)
base_circle = circle_surface(100, BLACK)
base_square_red = square_surface(100, RED)

clock = pygame.time.Clock()
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    win.fill(BG_COLOR)

    pygame.draw.rect(win, BLACK, (40, 30, 520, 520), 4)

    big_circle = pygame.transform.scale(base_circle, (155, 155))
    win.blit(big_circle, (90, 70))

    yellow_square = square_surface(80, YELLOW)
    win.blit(yellow_square, (128, 108))

    green_square = square_surface(155, GREEN)
    win.blit(green_square, (330, 70))

    cut_triangle = triangle_surface(100, BG_COLOR)
    cut_triangle = pygame.transform.scale(cut_triangle, (155, 78))
    win.blit(cut_triangle, (330, 147))

    top_triangle = pygame.transform.scale(base_triangle, (60, 60))
    top_triangle = pygame.transform.flip(top_triangle, False, True)
    win.blit(top_triangle, (150, 300))

    mid_rect = pygame.transform.scale(base_square, (120, 60))
    win.blit(mid_rect, (120, 360))

    bottom_triangle = pygame.transform.scale(base_triangle, (60, 60))
    win.blit(bottom_triangle, (150, 420))

    z_width = 180
    z_height = 120
    thickness = 8
    z_x = 330
    z_y = 305

    top_bar = pygame.transform.scale(base_square_red, (z_width, thickness))
    win.blit(top_bar, (z_x, z_y))

    bottom_bar = pygame.transform.scale(base_square_red, (z_width, thickness))
    win.blit(bottom_bar, (z_x, z_y + z_height))

    diag_len = int(math.hypot(z_width, z_height))
    diag_surf = pygame.transform.scale(base_square_red, (diag_len, thickness))
    angle = math.degrees(math.atan2(z_height, z_width))
    diag_rot = pygame.transform.rotate(diag_surf, angle)

    center_x = z_x + z_width // 2
    center_y = z_y + z_height // 2 + thickness // 2
    diag_rect = diag_rot.get_rect(center=(center_x, center_y))
    win.blit(diag_rot, diag_rect.topleft)

    pygame.display.flip()
    clock.tick(60)

pygame.quit()
