#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <map>
#include <deque>
#include <algorithm>
#include <atomic>
using namespace std;
using namespace std::this_thread;

const int BOARD_SIZE = 10;
atomic<char> direction('r');

class TerminalMode
{
    struct termios oldt;

public:
    TerminalMode()
    {
        struct termios newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    ~TerminalMode()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
};

bool is_occupied(const pair<int, int> &cell, const deque<pair<int, int>> &snake)
{
    return find(snake.begin(), snake.end(), cell) != snake.end();
}

pair<int, int> spawn_food(const deque<pair<int, int>> &snake)
{
    pair<int, int> food;
    do
    {
        food = make_pair(rand() % BOARD_SIZE, rand() % BOARD_SIZE);
    } while (is_occupied(food, snake));
    return food;
}

void input_handler()
{
    TerminalMode mode;
    map<char, char> keymap = {{'d', 'r'}, {'a', 'l'}, {'w', 'u'}, {'s', 'd'}, {'q', 'q'}};
    while (true)
    {
        char input = getchar();
        if (keymap.find(input) != keymap.end())
        {
            if (input == 'q')
                exit(0);
            direction.store(keymap[input]);
        }
    }
}

void render_board(int size, const deque<pair<int, int>> &snake, pair<int, int> food)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (i == food.first && j == food.second)
            {
                cout << "🍎";
            }
            else if (is_occupied(make_pair(i, j), snake))
            {
                cout << "🐍";
            }
            else
            {
                cout << "⬜";
            }
        }
        cout << endl;
    }
}

void render_hud(const deque<pair<int, int>> &snake)
{
    cout << "length of snake: " << snake.size() << endl;
}

pair<int, int> get_next_head(pair<int, int> current, char dir)
{
    pair<int, int> next;
    if (dir == 'r')
    {
        next = make_pair(current.first, (current.second + 1) % BOARD_SIZE);
    }
    else if (dir == 'l')
    {
        next = make_pair(current.first, current.second == 0 ? BOARD_SIZE - 1 : current.second - 1);
    }
    else if (dir == 'd')
    {
        next = make_pair((current.first + 1) % BOARD_SIZE, current.second);
    }
    else if (dir == 'u')
    {
        next = make_pair(current.first == 0 ? BOARD_SIZE - 1 : current.first - 1, current.second);
    }
    return next;
}

bool check_self_collision(const pair<int, int> &head, const deque<pair<int, int>> &snake)
{
    return is_occupied(head, snake);
}

void move_snake(deque<pair<int, int>> &snake, const pair<int, int> &head, bool grow)
{
    snake.push_back(head);
    if (!grow)
        snake.pop_front();
}

void game_play()
{
    system("clear");
    deque<pair<int, int>> snake;
    snake.push_back(make_pair(0, 0));
    pair<int, int> food = spawn_food(snake);
    pair<int, int> head = make_pair(0, 1);

    while (true)
    {
        cout << "\033[H";
        if (check_self_collision(head, snake))
        {
            system("clear");
            cout << "Game Over" << endl;
            exit(0);
        }

        bool ate = (head == food);
        if (ate)
            food = spawn_food(snake);
        move_snake(snake, head, ate);

        render_board(BOARD_SIZE, snake, food);
        render_hud(snake);

        sleep_for(500ms);
        head = get_next_head(head, direction.load());
    }
}
