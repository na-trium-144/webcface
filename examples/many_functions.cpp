#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <webcface/webcface.hpp>
// kbhit() : return true if some key is pressed
// https://stackoverflow.com/questions/29335758/using-kbhit-and-getch-on-linux
#include <ncurses.h> // for kbhit()
#include <sys/ioctl.h>
#include <termios.h>
bool kbhit() {
  termios term;
  tcgetattr(0, &term);
  termios term2 = term;
  term2.c_lflag &= ~ICANON;
  tcsetattr(0, TCSANOW, &term2);
  int byteswaiting;
  ioctl(0, FIONREAD, &byteswaiting);
  tcsetattr(0, TCSANOW, &term);
  return byteswaiting > 0;
}

void hoge(int x) { std::cout << x << std::endl; }

#include <ctime>
#include <iostream>
#include <unistd.h>

std::string random_str(const int len) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

int main() {
  WebCFace::initStdLogger();
  int t = 0;
  WebCFace::startServer(3001);

  for (int i = 0; i < 100; i++) {
    const auto func_name = random_str(12);
    WebCFace::addFunctionToRobot(func_name, hoge, {"arg1"});
  }

  while (true) {
    {
      std::lock_guard l(WebCFace::callback_mutex);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      t++;
      WebCFace::sendData();
      if (kbhit()) {
        std::cout << "quit server....." << std::endl;
        WebCFace::quitServer();
        exit(1);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
}
