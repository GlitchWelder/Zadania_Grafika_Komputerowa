import math
import pygame

pygame.init()

WIDTH, HEIGHT = 600, 600
CENTER = (WIDTH // 2, HEIGHT // 2)
RADIUS = 150
N = 6

BG_COLOR = (255, 255, 0)
POLY_COLOR = (24, 69, 200)
OUTLINE_COLOR = (15, 15, 15)
TEXT_COLOR = (0, 0, 0)

win = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Zadanie 1")


def mat_mul(a, b):
    return [
        [a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0],
         a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1],
         a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2]],
        [a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0],
         a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1],
         a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2]],
        [a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0],
         a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1],
         a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2]],
    ]


def rotation(deg):
    rad = math.radians(deg)
    c = math.cos(rad)
    s = math.sin(rad)
    return [[c, -s, 0], [s, c, 0], [0, 0, 1]]


def scale(sx, sy):
    return [[sx, 0, 0], [0, sy, 0], [0, 0, 1]]


def shear(shx, shy):
    return [[1, shx, 0], [shy, 1, 0], [0, 0, 1]]


def reflect_x():
    return [[1, 0, 0], [0, -1, 0], [0, 0, 1]]


def reflect_y():
    return [[-1, 0, 0], [0, 1, 0], [0, 0, 1]]


def translate(tx, ty):
    return [[1, 0, tx], [0, 1, ty], [0, 0, 1]]


def transform_point(point, m):
    x, y = point
    tx = m[0][0] * x + m[0][1] * y + m[0][2]
    ty = m[1][0] * x + m[1][1] * y + m[1][2]
    return (int(CENTER[0] + tx), int(CENTER[1] + ty))


def make_regular_polygon(n, radius):
    pts = []
    start_angle = -math.pi / 2
    for i in range(n):
        ang = start_angle + (2 * math.pi * i) / n
        pts.append((radius * math.cos(ang), radius * math.sin(ang)))
    return pts


BASE_POLYGON = make_regular_polygon(N, RADIUS)


TRANSFORMS = {
    pygame.K_1: scale(0.35, 0.35),
    pygame.K_2: mat_mul(rotation(-44), scale(0.62, 0.62)),
    pygame.K_3: mat_mul(reflect_x(), scale(0.34, 0.78)),
    pygame.K_4: mat_mul(shear(-0.32, 0.0), scale(0.86, 0.74)),
    pygame.K_5: mat_mul(translate(0, -220), scale(0.92, 0.24)),
    pygame.K_6: mat_mul(shear(0.0, 0.52), scale(0.36, 0.90)),
    pygame.K_7: mat_mul(mat_mul(reflect_x(), reflect_y()), scale(0.34, 0.78)),
    pygame.K_8: mat_mul(translate(-130, 160), mat_mul(rotation(-28), scale(0.86, 0.30))),
    pygame.K_9: mat_mul(translate(160, 20), mat_mul(mat_mul(shear(0.12, 0.0), reflect_x()), scale(1.06, 0.40))),
}

current_matrix = TRANSFORMS[pygame.K_1]

font = pygame.font.SysFont("consolas", 18)

clock = pygame.time.Clock()
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN and event.key in TRANSFORMS:
            current_matrix = TRANSFORMS[event.key]

    win.fill(BG_COLOR)

    transformed = [transform_point(p, current_matrix) for p in BASE_POLYGON]
    pygame.draw.polygon(win, POLY_COLOR, transformed)
    pygame.draw.polygon(win, OUTLINE_COLOR, transformed, 3)

    info = font.render("Klawisze 1-9: wybierz wariant", True, TEXT_COLOR)
    win.blit(info, (18, 565))

    pygame.display.update()
    clock.tick(60)

pygame.quit()
