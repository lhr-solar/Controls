import curses
import threading
import getch

# Globals
stdscr = None
lights = {"BPS_RDY":0, "ARY_RDY":0, "BPS:FLT":0, "CTRL_FLT":0, "LT":0, "RT":0, "MTR":0, "HDLT":0}

def display():
    """
    Displays a simulated version of the steering wheel
    and accelerator and brake pedals
    """
    global stdscr
    ### Steering Wheel ###
    # A
    stdscr.addstr(2, 10, "------------")
    stdscr.addstr(3, 10, "| << || >> |")
    stdscr.addstr(4, 10, "------------")
    # B
    stdscr.addstr(11, 35, "| FWD/REV |")
    stdscr.addstr(12, 35, "-----------")
    # C
    stdscr.addstr(7, 51, " CRS EN  |")
    stdscr.addstr(8, 51, "----------")
    # D
    stdscr.addstr(5, 51, " CRS SET |")
    stdscr.addstr(6, 51, "----------")
    # E
    stdscr.addstr(3, 62, "| REGEN |")
    stdscr.addstr(4, 62, "---------")
    # F
    stdscr.addstr(0, 0, "---------")
    stdscr.addstr(1, 0, "| RADIO |")
    stdscr.addstr(2, 0, "---------")
    # G
    stdscr.addstr(5, 10, "| BPS ARY  |")
    stdscr.addstr(6, 10, "| BPS CTRL |")
    stdscr.addstr(7, 10, "| LT  RT   |")
    stdscr.addstr(8, 10, "| MTR HDLT |")
    stdscr.addstr(9, 10, "------------")
    # H
    stdscr.addstr(2, 51, "----------")
    stdscr.addstr(3, 51, "   HZD   |")
    stdscr.addstr(4, 51, "----------")
    # I
    stdscr.addstr(0, 62, "--------")
    stdscr.addstr(1, 62, "| HORN |")
    stdscr.addstr(2, 62, "--------")
    # J
    stdscr.addstr(11, 25, "| HDLT |")
    stdscr.addstr(12, 25, "--------")
    # Center
    stdscr.addstr(2, 22, "-----------------------------")
    stdscr.addstr(3, 50, "|")
    stdscr.addstr(4, 50, "|")
    stdscr.addstr(5, 50, "|")
    stdscr.addstr(6, 50, "|")
    stdscr.addstr(7, 50, "|")
    stdscr.addstr(8, 22, "-----------------------------")
    stdscr.addstr(9, 22, "          Rear View         |")
    stdscr.addstr(10, 22, "----------------------------")

    ### Pedals ###
    stdscr.refresh()


def generate():
    """
    Generates all hardware simulated data
    """
    pass


def screen_init():
    global stdscr
    stdscr = curses.initscr()
    curses.start_color()
    curses.noecho()
    curses.cbreak()


def screen_end():
    curses.echo()
    curses.nocbreak()
    curses.endwin()
    stdscr = None


def user_input():
    """
    Gets user input to alter the state of the display
    """
    while True:
        ch = getch.getch().lower()
        if ch == 'q':
            screen_end()
            exit()
        elif ch == 'r':
            lights["RT"] = 1
        stdscr.addstr(13, 0, ch)


def engine():
    """
    Main engine for the simulator
    """
    screen_init()
    while True:
        try:
            display()
        except KeyboardInterrupt:
            screen_end()
            exit()
        except Exception as e:
            screen_end()
            print(e)


if __name__ == "__main__":
    engine = threading.Thread(target=engine, daemon=True)
    user_input = threading.Thread(target=user_input, daemon=True)
    engine.start()
    user_input.start()
    screen_end()
    exit()
