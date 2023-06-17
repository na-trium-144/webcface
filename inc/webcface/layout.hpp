#pragma once
#include <webcface/server.hpp>
#include <webcface/registration.hpp>
#include <jsoncpp/json/json.h>
#include <string>
#include <vector>
#include <concepts>

namespace WebCFace
{
inline namespace Layout
{
struct Component {
    Component() : json(Json::objectValue) { json["type"] = "None"; }

    //! 複数のComponentを普通に横に並べて表示(Vector)
    /*! Componentを並べたinitializer_listをComponentとして扱うためのコンストラクタです
     * explicitではない\n
     * `Vector`は省略して、単に二重波括弧で囲えばVectorになります\n
     * `{{ component, component, component, ...}}`
     * \param vec Componentのリスト
     */
    Component(const std::vector<Component>& vec) : Component()
    {
        json["type"] = "Vector";
        Json::Value children(Json::arrayValue);
        for (const auto& c : vec) {
            children.append(c.json);
        }
        json["children"] = children;
    }

    //! 文字列を表示 or 変数の値を表示
    /*! DisplayableをComponentとして扱うためのコンストラクタ
     * explicitではない
     * \sa WebCFace::Registration::Displayable
     */
    template <std::convertible_to<Displayable> T>
    Component(const T& value) : Component()
    {
        json["type"] = "Value";
        json["value"] = Displayable(value).json;
    }

    // operator Json::Value() const { return json; }


    //! 実際にフロントエンドに送るデータ
    //! コンストラクタ内で生成
    Json::Value json;
};

struct Alert : Component {
    explicit Alert(const Component& c, const std::string& severity = "error") : Component()
    {
        json["type"] = "Alert";
        json["severity"] = severity;
        json["text"] = c.json;
    }
};

//! Componentを横に並べる
/*! Vectorは省略して、単に二重波括弧で囲えばVectorになります
 * \sa Component のコンストラクタ
 */
struct Vector : Component {
    //! 複数のComponentを横に並べて表示
    /*!
     * \param vec Componentのリスト
     */
    explicit Vector(const std::vector<Component>& vec) : Component(vec) {}
    //! PyVectorからの変換用
    explicit Vector(const Component& vec) : Component(vec) {}
};

//! Componentを縦に並べる
struct Stack : Component {
    //! 複数のComponentを縦に並べて表示
    /*! インラインでこれを書くと読みにくくなるのでStack型の変数に入れてから使っても良さそう\n
     * Stack{{\n
     *     component, \n
     *     component,\n
     *     ...\n
     *   }}\n
     * \param vec Componentのリスト
     */
    Stack(const std::vector<Component>& vec) : Component(vec) { json["type"] = "Stack"; }
    //! PyStackからの変換用
    explicit Stack(const Component& vec) : Component(vec) { json["type"] = "Stack"; }
};

//! 関数実行ボタン
/*! ボタンを表示する\n
 * 指定した関数が存在しない場合ボタンは無効化されグレーになる\n
 * プロキシチェーン式にオプションを指定できます
 * 例: Button("name", "callback"_callback).MuiColor("primary")...
 * 関数に引数を渡して実行したい場合は WebCFace::Registration::RegisterCallback::arg()
 * で指定してください
 */
struct Button : Component {
    //! ボタンの名前を指定したボタン
    /*!
     * \param display_name ボタンに表示する名前
     * \param callback 実行する関数
     */
    Button(const Displayable& display_name, const RegisterCallback& callback) : Component()
    {
        json["type"] = "Button";
        json["display_name"] = display_name.json;
        json["callback"] = callback.json();
    }
    //! ボタンの名前を省略したボタン
    /*! callbackの名前がボタンの名前として表示されます
     * \param callback 実行する関数
     */
    explicit Button(const RegisterCallback& callback) : Button(callback.callback_name, callback) {}
    //! ボタンの色を変更
    /*!
     * \param mui_color 色\n
     * "primary", "secondary", "error", "warning", "info", "status"が使用可能\n
     */
    Button& MuiColor(const Displayable& mui_color)
    {
        json["color"] = mui_color.json;
        return *this;
    }
};
struct Drawing;
struct DrawingLayer;
struct DrawingComponent {
    DrawingLayer* parent;
    int id;
    explicit DrawingComponent(DrawingLayer* parent, int id) : parent(parent), id(id) {}

    DrawingComponent& onClick(const RegisterCallback& callback);
};
struct DrawingLayer {
    std::string name;
    explicit DrawingLayer(const std::string& name) : name(name) {}
    DrawingComponent drawLine(const Displayable& x, const Displayable& y, const Displayable& x2,
        const Displayable& y2, const Displayable& color)
    {
        Json::Value dc{Json::objectValue};
        dc["type"] = "Line";
        dc["x"] = x.json;
        dc["y"] = y.json;
        dc["x2"] = x2.json;
        dc["y2"] = y2.json;
        dc["color"] = color.json;
        return DrawingComponent(this, addToParent(dc));
    }
    DrawingComponent drawRect(const Displayable& x, const Displayable& y, const Displayable& x2,
        const Displayable& y2, const Displayable& color)
    {
        Json::Value dc{Json::objectValue};
        dc["type"] = "Rect";
        dc["x"] = x.json;
        dc["y"] = y.json;
        dc["x2"] = x2.json;
        dc["y2"] = y2.json;
        dc["color"] = color.json;
        return DrawingComponent(this, addToParent(dc));
    }
    DrawingComponent drawCircle(
        const Displayable& x, const Displayable& y, const Displayable& r, const Displayable& color)
    {
        Json::Value dc{Json::objectValue};
        dc["type"] = "Circle";
        dc["x"] = x.json;
        dc["y"] = y.json;
        dc["r"] = r.json;
        dc["color"] = color.json;
        return DrawingComponent(this, addToParent(dc));
    }
    DrawingComponent drawText(const Displayable& x, const Displayable& y,
        const Displayable& font_size, const Displayable& text, const Displayable& color)
    {
        Json::Value dc{Json::objectValue};
        dc["type"] = "Text";
        dc["x"] = x.json;
        dc["y"] = y.json;
        dc["font_size"] = font_size.json;
        dc["text"] = text.json;
        dc["color"] = color.json;
        return DrawingComponent(this, addToParent(dc));
    }
    int addToParent(const Json::Value& dc)
    {
        std::lock_guard lock(internal_mutex);
        drawing_layer[name].append(dc);
        return static_cast<int>(drawing_layer[name].size()) - 1;
    }
};
struct Drawing : Component {
    Drawing(const Displayable& width, const Displayable& height) : Component()
    {
        json["type"] = "Drawing";
        json["width"] = width.json;
        json["height"] = height.json;
        json["layers"] = Json::arrayValue;
    }
    void addLayer(const std::string& name) { json["layers"].append(name); }
    DrawingLayer createLayer(const std::string& name)
    {
        std::lock_guard lock(internal_mutex);
        bool is_new = (drawing_layer.find(name) == drawing_layer.end());
        drawing_layer[name] = Json::arrayValue;
        // if (is_new) {
        //     setting_changed = true;
        // }
        addLayer(name);
        return DrawingLayer(name);
    }
};

inline DrawingComponent& DrawingComponent::onClick(const RegisterCallback& callback)
{
    std::lock_guard lock(internal_mutex);
    drawing_layer[parent->name][id]["on_click"] = callback.json();
    return *this;
}

inline void addPageLayoutJson(const std::string& name, const Json::Value& layout)
{
    std::lock_guard lock(internal_mutex);
    bool is_new = (custom_page_layout.find(name) == custom_page_layout.end());
    custom_page_layout[name] = layout;
    // if (is_new) {
    //     setting_changed = true;
    // }
}
inline void addPageLayoutJson(const std::string& name, const std::string& layout)
{
    Json::Value parsed(Json::objectValue);
    Json::Reader reader;
    reader.parse(layout, parsed);
    addPageLayoutJson(name, parsed);
}
//! カスタムページの追加
/*! レイアウトをjsonで表現したものを登録する
 *  カスタムページの概要は 2_5_custom_page.md を見てください
 * \param name ページ名
 * \param layout レイアウト
 */
inline void addPageLayout(const std::string& name, const Layout::Stack& layout)
{
    Json::Value layoutjson = Json::arrayValue;
    layoutjson.append(layout.json);
    addPageLayoutJson(name, layoutjson);
}

struct PageLayout {
    std::string name;
    explicit PageLayout(const std::string& name) : name(name)
    {
        {
            std::lock_guard lock(internal_mutex);
            // if (custom_page_layout.find(name) == custom_page_layout.end()) {
            //     setting_changed = true;
            // }
        }
        clear();
    }
    void clear()
    {
        std::lock_guard lock(internal_mutex);
        custom_page_layout[name] = Json::arrayValue;
    }
    PageLayout& add(const Component& c)
    {
        std::lock_guard lock(internal_mutex);

        custom_page_layout[name].append(c.json);
        return *this;
    }
    PageLayout& newLine()
    {
        std::lock_guard lock(internal_mutex);

        Json::Value endl{Json::objectValue};
        endl["type"] = "br";
        custom_page_layout[name].append(endl);
        return *this;
    }

    struct Endl {
    } endl;
    PageLayout& operator<<(const Component& c)
    {
        add(c);
        return *this;
    }
    PageLayout& operator<<(Endl)
    {
        newLine();
        return *this;
    }
};

}  // namespace Layout
inline void dialog(const Layout::PageLayout& page)
{
    dialog(page.name);
}
}  // namespace WebCFace
