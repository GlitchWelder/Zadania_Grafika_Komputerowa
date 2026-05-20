"""
Wymagania: zainstalowany pygame (pip install pygame).
Uruchomienie: python kolko_krzyzyk.py
"""

import sys
import pygame

# inicjalizacja biblioteki pygame
pygame.init()

# rozmiar okna
WIDTH = 600
HEIGHT = 600

# kolory w formacie RGB 
LINE_COLOR = (23, 145, 135)   # kolor linii siatki (ciemniejszy turkus)
BG_COLOR = (28, 170, 156)     # kolor tła (jasny turkus)
X_COLOR = (84, 84, 84)        # kolor krzyżyków (ciemnoszary)
O_COLOR = (242, 235, 211)     # kolor kółek (kremowy)

LINE_WIDTH = 10               # grubość linii w pikselach
CELL_SIZE = WIDTH // 3        # rozmiar pojedynczej komórki (dzielimy planszę na 3 równe kolumny)

# tworzymy okno gry i ustawiamy tytuł
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption('Kółko i krzyż')

# dwa rozmiary czcionek - mała do wyświetlania aktualnego gracza, duża do komunikatu końcowego
font = pygame.font.SysFont(None, 40)
bigfont = pygame.font.SysFont(None, 60)

# plansza 3x3 jako lista list, None oznacza puste pole
# board[row][col] przechowuje 'X', 'O' albo None
board = [[None for _ in range(3)] for _ in range(3)]

current_player = 'X'   # zaczynamy od gracza X
game_over = False       # flaga - czy gra się już skończyła
winner = None           # kto wygrał (albo 'Draw' jeśli remis)


def draw_lines():
    # rysuje tło i siatkę planszy (2 pionowe + 2 poziome linie)
    screen.fill(BG_COLOR)
    # pionowa linia po 1/3 szerokości
    pygame.draw.line(screen, LINE_COLOR, (CELL_SIZE, 0), (CELL_SIZE, HEIGHT), LINE_WIDTH)
    # pionowa linia po 2/3 szerokości
    pygame.draw.line(screen, LINE_COLOR, (CELL_SIZE * 2, 0), (CELL_SIZE * 2, HEIGHT), LINE_WIDTH)
    # pozioma linia po 1/3 wysokości
    pygame.draw.line(screen, LINE_COLOR, (0, CELL_SIZE), (WIDTH, CELL_SIZE), LINE_WIDTH)
    # pozioma linia po 2/3 wysokości
    pygame.draw.line(screen, LINE_COLOR, (0, CELL_SIZE * 2), (WIDTH, CELL_SIZE * 2), LINE_WIDTH)


def draw_marks():
    # rysuje wszystkie X-y i O-iki na planszy
    offset = CELL_SIZE // 4  # o ile pikseli odsuwamy symbol od środka komórki

    for row in range(3):
        for col in range(3):
            mark = board[row][col]

            # środek aktualnej komórki w pikselach
            x = col * CELL_SIZE + CELL_SIZE // 2
            y = row * CELL_SIZE + CELL_SIZE // 2

            if mark == 'X':
                # krzyżyk = dwie linie na krzyż (ukośne)
                start1 = (x - offset, y - offset)  # lewy górny
                end1   = (x + offset, y + offset)  # prawy dolny
                start2 = (x - offset, y + offset)  # lewy dolny
                end2   = (x + offset, y - offset)  # prawy górny
                pygame.draw.line(screen, X_COLOR, start1, end1, LINE_WIDTH)
                pygame.draw.line(screen, X_COLOR, start2, end2, LINE_WIDTH)
            elif mark == 'O':
                # kółko - okrąg narysowany tylko jako obramowanie (LINE_WIDTH = grubość)
                pygame.draw.circle(screen, O_COLOR, (x, y), offset, LINE_WIDTH)


def available_square(row, col):
    # zwraca True jeśli pole jest puste (można tam postawić symbol)
    return board[row][col] is None


def mark_square(row, col, player):
    # wpisuje symbol gracza ('X' lub 'O') na dane pole planszy
    board[row][col] = player


def check_winner():
    # sprawdza wszystkie możliwe linie wygranej i zwraca zwycięzcę (lub None / 'Draw')
    lines = []

    # dodajemy 3 wiersze
    for i in range(3):
        lines.append(board[i])

    # dodajemy 3 kolumny (trzeba ręcznie zebrać elementy z każdej kolumny)
    for i in range(3):
        lines.append([board[0][i], board[1][i], board[2][i]])

    # dodajemy 2 przekątne
    lines.append([board[0][0], board[1][1], board[2][2]])  # od lewego górnego
    lines.append([board[0][2], board[1][1], board[2][0]])  # od prawego górnego

    # sprawdzamy czy jakaś linia ma 3 takie same niepuste symbole
    for line in lines:
        if line[0] and line[0] == line[1] == line[2]:
            return line[0]  # zwracamy 'X' albo 'O'

    # jeśli żaden nie wygrał, sprawdzamy czy jest jeszcze wolne pole
    for row in board:
        for cell in row:
            if cell is None:
                return None  # gra trwa dalej

    return 'Draw'  # brak wolnych pól i brak zwycięzcy = remis


def restart():
    # resetuje stan gry do początkowego (używamy global bo modyfikujemy zmienne globalne)
    global board, current_player, game_over, winner
    board = [[None for _ in range(3)] for _ in range(3)]
    current_player = 'X'
    game_over = False
    winner = None


def draw_status():
    # wyświetla informacje na ekranie
    if game_over:
        if winner == 'Draw':
            text = 'Remis!'
        else:
            text = f'Wygrał {winner}!'

        # renderujemy tekst jako Surface i wyśrodkowujemy na ekranie
        msg = bigfont.render(text, True, (255, 255, 255))
        rect = msg.get_rect(center=(WIDTH // 2, HEIGHT // 2))
        screen.blit(msg, rect)  # blit = naklejamy tekst na ekran

        sub = font.render('Naciśnij R aby zagrać ponownie', True, (255, 255, 255))
        rect2 = sub.get_rect(center=(WIDTH // 2, HEIGHT // 2 + 50))
        screen.blit(sub, rect2)
    else:
        # podczas gry pokazujemy tylko który gracz ma teraz ruch
        text = f'Ruch: {current_player}'
        msg = font.render(text, True, (255, 255, 255))
        screen.blit(msg, (10, HEIGHT - 40))  # w lewym dolnym rogu


# clock do ograniczenia FPS - bez tego gra mogłaby zjeść 100% CPU
clock = pygame.time.Clock()

# główna pętla gry - wykonuje się 60 razy na sekundę
while True:
    # obsługa eventów
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            # użytkownik zamknął okno - kończymy program
            pygame.quit()
            sys.exit()

        if event.type == pygame.MOUSEBUTTONDOWN and not game_over:
            # gracz kliknął myszką podczas trwającej gry
            mx, my = pygame.mouse.get_pos()  # pozycja kursora w pikselach

            # przeliczamy piksel na indeks wiersza/kolumny siatki (dzielenie całkowite)
            row = my // CELL_SIZE
            col = mx // CELL_SIZE

            # upewniamy się że kliknięto w granicach planszy
            if 0 <= row < 3 and 0 <= col < 3:
                if available_square(row, col):
                    mark_square(row, col, current_player)

                    result = check_winner()
                    if result:
                        # ktoś wygrał albo remis - kończymy grę
                        game_over = True
                        winner = result
                    else:
                        # zmiana tury: jeśli był X to teraz O, i odwrotnie
                        current_player = 'O' if current_player == 'X' else 'X'

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_r:
                # gracz wcisnął R - restartujemy grę
                restart()

    # rysujemy klatke: najpierw siatka, potem symbole, potem tekst statusu
    draw_lines()
    draw_marks()
    draw_status()

    # odświeżamy ekran (bez tego nic nie byłoby widać)
    pygame.display.update()

    # czekamy tyle żeby nie przekroczyć 60 klatek na sekundę
    clock.tick(60)
