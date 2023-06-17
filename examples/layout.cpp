#include <webcface/layout.hpp>
#include <iostream>
void test(){
    std::cout << "test" << std::endl;
}
int main()
{
    using namespace WebCFace::Layout;
    using namespace WebCFace::Literals;
    // clang-format off
    Stack s{{
        {{ Button("hoge", "fuga"_callback), Button("hoge", "fuga"_callback), }},
        Button("hoge", "fuga"_callback),
        Button("fuga"_callback)
    }};
    WebCFace::addPageLayout("test", {{
        {{ Button("hoge", "fuga"_callback), Button("fuga"_callback).MuiColor([]{return "secondary";}), s }},
        {{ Button("hoge", test), }},
    }});
    // clang-format on
}
