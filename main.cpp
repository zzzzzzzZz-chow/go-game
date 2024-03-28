#include <chrono>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/screen/string.hpp>
#include <vector>

ftxui::Component QiPan() {
  using namespace ftxui;
  auto state = Make<std::vector<std::vector<int>>>();
  auto next = Make<int>(1);
  for (int i = 0; i < 19; ++i) {
    std::vector<int> tmp;
    for (int j = 0; j < 19; ++j)
      tmp.push_back({0});
    state->push_back(tmp);
  }

  auto con = ftxui::Container::Vertical({});

  auto flush_con = [=] {
    con->DetachAllChildren();
    for (int i = 0; i < 19; ++i) {

      auto con1 = ftxui::Container::Horizontal({});
      for (int j = 0; j < 19; ++j) {
        auto bo = ButtonOption{};

        if ((*state)[i][j] == 0) {
          AnimatedColorOption ba;
          ba.Set(Color::GreenYellow,
                 (*next) == 1 ? Color::White : Color::Black);
          AnimatedColorOption fo;
          fo.Set(Color::Red, Color::Blue);

          if (i == 3 && (j == 3 || j == 9 || j == 15) ||
              i == 9 && (j == 3 || j == 9 || j == 15) ||
              i == 15 && (j == 3 || j == 9 || j == 15)) {
            ba.inactive = Color::Yellow;
          }

          bo.animated_colors = {ba, fo};
        } else if ((*state)[i][j] > 0) {
          // 白棋
          AnimatedColorOption white_go;
          white_go.Set(Color::White, Color::White);
          AnimatedColorOption white_go_txt;
          white_go_txt.Set(Color::Black, Color::Black);
          bo.animated_colors = {white_go, white_go_txt};
        } else {
          // 黑棋
          AnimatedColorOption black_go;
          black_go.Set(Color::Black, Color::Black);
          AnimatedColorOption black_go_txt;
          black_go_txt.Set(Color::White, Color::White);
        }

        bo.transform = [=](const EntryState &es) {
          if ((*state)[i][j] == 0)
            return text("");
          return text(std::to_string(abs((*state)[i][j]))) | align_right |
                 color((*state)[i][j] < 0 ? Color::White : Color::Black);
        };
        bo.on_click = [] {};
        con1->Add(ftxui::Button(bo) |
                  ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 2) |
                  ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1));
      }
      con->Add(con1);
    }
  };
  flush_con();
  std::function<void()> flush_state = [state, &flush_state] {
    // 定义方向数组，用于表示上、下、左、右四个方向
    const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    // 深度优先搜索函数
    std::function<void(std::vector<std::vector<int>> &,
                       std::vector<std::vector<bool>> &, int, int, int &)>
        dfs = [&directions, &dfs](std::vector<std::vector<int>> &board,
                                  std::vector<std::vector<bool>> &visited,
                                  int row, int col, int &liberties) {
          int m = board.size();
          int n = board[0].size();

          // 检查当前位置是否超出边界，或者已经访问过
          if (row < 0 || row >= m || col < 0 || col >= n || visited[row][col])
            return;

          visited[row][col] = true; // 标记当前位置为已访问

          // 如果当前位置是空位，则增加气的数量
          if (board[row][col] == 0) {
            liberties++;
            return;
          }

          // 如果当前位置有棋子，则继续搜索其周围的位置
          for (auto &dir : directions) {
            int newRow = row + dir[0];
            int newCol = col + dir[1];
            if (newRow >= 0 && newRow < m && newCol >= 0 && newCol < n &&
                !visited[newRow][newCol]) {
              // 如果相邻位置的棋子颜色相同，则递归搜索
              if (board[newRow][newCol] * board[row][col] >= 0) {
                dfs(board, visited, newRow, newCol, liberties);
              }
            }
          }
        };

    // 计算每个棋子的气的数量
    auto countLiberties = [&dfs, state](std::vector<std::vector<int>> &board)
        -> std::vector<std::vector<int>> {
      int m = board.size();
      int n = board[0].size();
      std::vector<std::vector<int>> result;

      // 遍历棋盘上的每个位置
      for (int i = 0; i < m; ++i) {
        std::vector<int> tmp;
        for (int j = 0; j < n; ++j) {
          // 如果当前位置是有棋子的
          if (board[i][j] != 0) {
            int liberties = 0;
            std::vector<std::vector<bool>> visited(m,
                                                   std::vector<bool>(n, false));
            dfs(board, visited, i, j, liberties);
            if ((*state)[i][j] < 0)
              liberties *= -1;
            tmp.push_back(liberties);
            continue;
          }
          tmp.push_back(0);
        }
        result.push_back(tmp);
      }
      return result;
    };

  re:
    // 更新棋子的气
    auto s = countLiberties(*state);
    bool reflush = false;
    for (int i = 0; i < 19; ++i)
      for (int j = 0; j < 19; ++j) {
        if (s[i][j] == 0 && (*state)[i][j] != 0) // 有吃子
          reflush = true;
        (*state)[i][j] = s[i][j];
      }
    if (reflush) // 有吃子，需要重新计算一遍
      goto re;
  };

  con |= CatchEvent([=](Event e) -> bool {
    static auto time = std::chrono::steady_clock::now();
    if (!e.is_mouse())
      return false;
    auto mouse = e.mouse();
    if (mouse.motion == Mouse::Motion::Released)
      return false;
    if (auto old = time;
        time = std::chrono::steady_clock::now(),
        time - old < std::chrono::milliseconds(500)) { // 下棋间隔不可小于0.5s
      return true;
    }

    int &pos = (*state)[mouse.y - 2][(mouse.x - 8) / 2];
    if (pos != 0) // 有子
      return true;
    pos = *next;
    if (*next == 1)
      *next = -1;
    else
      *next = 1;
    flush_state();
    flush_con();
    return true;
  });
  return con;
}

int main() {
  using namespace ftxui;
  auto screen = ScreenInteractive::FitComponent();
  auto qipan = QiPan() | borderDouble;
  auto button = Button("Quit", screen.ExitLoopClosure()) | color(Color::Red);
  auto h = Container::Horizontal({button, qipan});
  auto layout = Renderer(h, [=] {
    return window(text("GO") | center | blink, h->Render()) |
           color(LinearGradient{}
                     .Angle(45)
                     .Stop(0xff0000_rgb, 0)
                     .Stop(0x00ff00_rgb, 0.5)
                     .Stop(0x0000ff_rgb, 1));
  });
  auto com = Renderer(layout, [=] { return layout->Render(); });
  screen.Loop(com);
  return 0;
}
