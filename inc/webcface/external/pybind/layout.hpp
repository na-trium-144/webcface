#pragma once
#include <webcface/layout.hpp>

namespace WebCFace::pybind
{
inline namespace Layout
{
using namespace WebCFace::Layout;
namespace py = pybind11;

Component convertPyComponent(const py::object& value);

struct PyComponent : Component {
    PyComponent() : Component() {}
    explicit PyComponent(const py::object& value) : Component(convertPyComponent(value)) {}
};
//! Componentを横に並べる
/*! Vectorは省略して、単に角括弧で囲ってリストにすればVectorになります
 */
struct PyVector : Vector {
    explicit PyVector(const py::list& vec) : Vector(convertPyComponent(vec)) {}
};
//! Componentを縦に並べる
struct PyStack : Stack {
    explicit PyStack(const py::list& vec) : Stack(convertPyComponent(vec)) {}
};

//! 関数実行ボタン
/*! ボタンを表示する\n
 * 指定した関数が存在しない場合ボタンは無効化されグレーになる\n
 */
struct PyButton : Button {
    //! ボタンの名前を指定したボタン
    /*!
     * \param display_name ボタンに表示する名前(PyDisplayable)
     * \param callback 実行する関数(PyRegisterCallback)
     * \sa mui_color()
     */
    PyButton(const py::object& display_name, const py::object& callback,
        const py::object& mui_color = py::none())
        : Button(PyDisplayable(display_name), PyRegisterCallback(callback))
    {
        if (!py::isinstance<py::none>(mui_color)) {
            this->MuiColor(mui_color);
        }
    }
    //! ボタンの名前を省略したボタン
    /*! callbackの名前がボタンの名前として表示されます
     * \param callback 実行する関数
     */
    explicit PyButton(const py::object& callback)
        : PyButton(py::str(PyRegisterCallback(callback).callback_name), callback)
    {
    }
    //! ボタンの色を変更
    /*! Button.mui_color(mui_color)
     * \param mui_color 色(PyDisplayable)\n
     * "primary", "secondary", "error", "warning", "info", "status"が使用可能\n
     */
    PyButton& MuiColor(const py::object& mui_color)
    {
        this->Button::MuiColor(PyDisplayable(mui_color));
        return *this;
    }
};
struct PyDrawing;
struct PyDrawingLayer;
struct PyDrawingComponent: public DrawingComponent{
    PyDrawingComponent(DrawingLayer* parent, int id): DrawingComponent(parent, id) {
    }
    PyDrawingComponent& onClick(const py::object& callback){
        this->DrawingComponent::onClick(PyRegisterCallback(callback));
        return *this;
    }
};
struct PyDrawingLayer : DrawingLayer {
    PyDrawingLayer(Drawing* parent, int id) : DrawingLayer(parent, id) {}
    PyDrawingComponent drawLine(const py::object& x, const py::object& y, const py::object& x2,
        const py::object& y2, const py::object& color)
    {
        const auto& dc = DrawingLayer::drawLine(PyDisplayable(x), PyDisplayable(y), PyDisplayable(x2),
            PyDisplayable(y2), PyDisplayable(color));
        return PyDrawingComponent(dc.parent, dc.id);
    }
    PyDrawingComponent drawRect(const py::object& x, const py::object& y, const py::object& x2,
        const py::object& y2, const py::object& color)
    {
        const auto& dc = DrawingLayer::drawRect(PyDisplayable(x), PyDisplayable(y), PyDisplayable(x2),
            PyDisplayable(y2), PyDisplayable(color));
        return PyDrawingComponent(dc.parent, dc.id);
    }
    PyDrawingComponent drawCircle(
        const py::object& x, const py::object& y, const py::object& r, const py::object& color)
    {
        const auto& dc = DrawingLayer::drawCircle(
            PyDisplayable(x), PyDisplayable(y), PyDisplayable(r), PyDisplayable(color));
        return PyDrawingComponent(dc.parent, dc.id);
    }
};

struct PyDrawing : Drawing {
    PyDrawing(const py::object& width, const py::object& height)
        : Drawing(PyDisplayable(width), PyDisplayable(height))
    {
    }
    PyDrawingLayer pyCreateLayer()
    {
        const auto& dl = createLayer();
        return PyDrawingLayer(dl.parent, dl.id);
    }
};
Component convertPyComponent(const py::object& value)
{
    if (py::isinstance<PyComponent>(value)) {
        return value.cast<PyComponent>();
    } else if (py::isinstance<PyVector>(value)) {
        return value.cast<PyVector>();
    } else if (py::isinstance<PyStack>(value)) {
        return value.cast<PyStack>();
    } else if (py::isinstance<PyButton>(value)) {
        return value.cast<PyButton>();
    } else if (py::isinstance<PyDrawing>(value)) {
        return value.cast<PyDrawing>();
    } else if (py::isinstance<py::list>(value)) {
        std::vector<Component> vec;
        for (std::size_t i = 0; i < py::len(value); i++) {
            vec.push_back(PyComponent(value.cast<py::list>()[i]));
        }
        return Component(vec);
    } else {
        return PyDisplayable(value);
    }
}


inline void addPageLayout(const std::string& name, const py::object& layout)
{
    if (py::isinstance<PyStack>(layout)) {
        addPageLayout(name, layout.cast<PyStack>());
    } else {
        addPageLayout(name, PyStack(layout));
    }
}

struct PyPageLayout : PageLayout {
    explicit PyPageLayout(const std::string& name) : PageLayout(name)
    {
        if (custom_page_layout.find(name) == custom_page_layout.end()) {
            setting_changed = true;
            clear();
        }
    }
    PyPageLayout& add(const py::object& c)
    {
        PageLayout::add(convertPyComponent(c));
        return *this;
    }
    PyPageLayout& newLine()
    {
        PageLayout::newLine();
        return *this;
    }
};

}  // namespace Layout
inline void defLayout(pybind11::module& m)
{
    using namespace pybind11::literals;
    py::class_<PyComponent>(m, "Component").def(py::init<const py::object&>());
    py::class_<PyVector>(m, "Vector").def(py::init<const py::object&>());
    py::class_<PyStack>(m, "Stack").def(py::init<const py::object&>());
    py::class_<PyButton>(m, "Button")
        .def(py::init<const py::object&, const py::object&, const py::object&>(), "name"_a,
            "callback"_a, "mui_color"_a = py::none())
        .def(py::init<const py::object&>(), "callback"_a)
        .def("mui_color", &PyButton::MuiColor);
    py::class_<PyDrawingComponent>(m, "DrawingComponent")
        .def("on_click", &PyDrawingComponent::onClick, "callback"_a);
    py::class_<PyDrawingLayer>(m, "DrawingLayer")
        .def("draw_line",
            py::overload_cast<const py::object&, const py::object&, const py::object&,
                const py::object&, const py::object&>(&PyDrawingLayer::drawLine),
            "x"_a, "y"_a, "x2"_a, "y2"_a, "color"_a)
        .def("draw_rect",
            py::overload_cast<const py::object&, const py::object&, const py::object&,
                const py::object&, const py::object&>(&PyDrawingLayer::drawRect),
            "x"_a, "y"_a, "x2"_a, "y2"_a, "color"_a)
        .def("draw_circle",
            py::overload_cast<const py::object&, const py::object&, const py::object&,
                const py::object&>(&PyDrawingLayer::drawCircle),
            "x"_a, "y"_a, "r"_a, "color"_a);
    py::class_<PyDrawing>(m, "Drawing")
        .def(py::init<const py::object&, const py::object&>(), "width"_a, "height"_a)
        .def("create_layer", &PyDrawing::pyCreateLayer);
    py::class_<PyPageLayout>(m, "PageLayout")
        .def(py::init<const std::string&>(), "name"_a)
        .def("clear", &PageLayout::clear)
        .def("add", &PyPageLayout::add)
        .def("new_line", &PyPageLayout::newLine);

    m.def("add_page_layout",
        pybind11::overload_cast<const std::string&, const py::object&>(&addPageLayout),
        "add Page Layout", "name"_a, "layout"_a);
}
}  // namespace WebCFace::pybind
