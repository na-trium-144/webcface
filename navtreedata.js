/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "WebCFace", "index.html", [
    [ "Links", "index.html#autotoc_md83", null ],
    [ "Installation", "index.html#autotoc_md84", [
      [ "Ubuntu 20.04, 22.04 (x86_64, arm64, armhf)", "index.html#autotoc_md85", null ],
      [ "Homebrew (MacOS, Linux)", "index.html#autotoc_md86", null ],
      [ "Windows", "index.html#autotoc_md87", null ],
      [ "Build from source", "index.html#autotoc_md88", [
        [ "Requirements", "index.html#autotoc_md89", null ],
        [ "Build (with Pure CMake)", "index.html#autotoc_md90", null ],
        [ "Build (with colcon, ROS2)", "index.html#autotoc_md91", null ],
        [ "WebUI", "index.html#autotoc_md92", null ],
        [ "tools", "index.html#autotoc_md93", null ]
      ] ]
    ] ],
    [ "Documentation", "index.html#autotoc_md94", null ],
    [ "License", "index.html#autotoc_md95", null ],
    [ "Tutorial", "md_00__tutorial.html", [
      [ "環境構築", "md_00__tutorial.html#autotoc_md1", null ],
      [ "Server", "md_00__tutorial.html#autotoc_md2", null ],
      [ "WebUI", "md_00__tutorial.html#autotoc_md3", null ],
      [ "データの送信", "md_00__tutorial.html#autotoc_md4", [
        [ "value", "md_00__tutorial.html#autotoc_md5", null ],
        [ "text", "md_00__tutorial.html#autotoc_md6", null ],
        [ "log", "md_00__tutorial.html#autotoc_md7", null ],
        [ "view", "md_00__tutorial.html#autotoc_md8", null ],
        [ "func", "md_00__tutorial.html#autotoc_md9", null ]
      ] ],
      [ "tools", "md_00__tutorial.html#autotoc_md10", null ],
      [ "Clientライブラリ", "md_00__tutorial.html#autotoc_md11", null ]
    ] ],
    [ "Client", "md_01__client.html", [
      [ "sync", "md_01__client.html#autotoc_md13", null ],
      [ "切断する", "md_01__client.html#autotoc_md14", null ],
      [ "ログ出力", "md_01__client.html#autotoc_md15", null ]
    ] ],
    [ "Member", "md_02__member.html", [
      [ "Field系クラスの扱いについて", "md_02__member.html#autotoc_md17", null ],
      [ "Event", "md_02__member.html#autotoc_md18", null ]
    ] ],
    [ "Value", "md_10__value.html", [
      [ "送信", "md_10__value.html#autotoc_md20", null ],
      [ "受信", "md_10__value.html#autotoc_md21", null ],
      [ "受信イベント", "md_10__value.html#autotoc_md22", null ]
    ] ],
    [ "Text", "md_11__text.html", [
      [ "送信", "md_11__text.html#autotoc_md24", null ],
      [ "受信", "md_11__text.html#autotoc_md25", null ],
      [ "受信イベント", "md_11__text.html#autotoc_md26", null ]
    ] ],
    [ "View", "md_13__view.html", [
      [ "送信", "md_13__view.html#autotoc_md28", null ],
      [ "ViewComponent", "md_13__view.html#autotoc_md29", [
        [ "text", "md_13__view.html#autotoc_md30", null ],
        [ "newLine", "md_13__view.html#autotoc_md31", null ],
        [ "button", "md_13__view.html#autotoc_md32", null ],
        [ "プロパティ", "md_13__view.html#autotoc_md33", null ]
      ] ],
      [ "受信", "md_13__view.html#autotoc_md34", null ],
      [ "受信イベント", "md_13__view.html#autotoc_md35", null ]
    ] ],
    [ "Image", "md_15__image.html", null ],
    [ "Func", "md_30__func.html", [
      [ "関数の登録", "md_30__func.html#autotoc_md38", [
        [ "引数", "md_30__func.html#autotoc_md39", null ],
        [ "hidden属性", "md_30__func.html#autotoc_md40", null ],
        [ "実行条件 (C++)", "md_30__func.html#autotoc_md41", null ]
      ] ],
      [ "関数の実行", "md_30__func.html#autotoc_md42", null ],
      [ "AnonymousFunc", "md_30__func.html#autotoc_md43", null ]
    ] ],
    [ "Log", "md_40__log.html", [
      [ "ログを出力する", "md_40__log.html#autotoc_md45", [
        [ "C++", "md_40__log.html#autotoc_md46", null ],
        [ "Python", "md_40__log.html#autotoc_md47", null ],
        [ "JavaScript", "md_40__log.html#autotoc_md48", null ]
      ] ],
      [ "ログを取得する", "md_40__log.html#autotoc_md49", null ],
      [ "受信イベント", "md_40__log.html#autotoc_md50", null ]
    ] ],
    [ "Message", "md_90__message.html", [
      [ "Sync", "md_90__message.html#autotoc_md52", [
        [ "sync init (kind = 80)", "md_90__message.html#autotoc_md53", null ],
        [ "svr version (kind = 88)", "md_90__message.html#autotoc_md54", null ],
        [ "ping (kind = 89)", "md_90__message.html#autotoc_md55", null ],
        [ "ping status (kind = 90)", "md_90__message.html#autotoc_md56", null ],
        [ "ping status req (kind = 91)", "md_90__message.html#autotoc_md57", null ],
        [ "sync (kind = 87)", "md_90__message.html#autotoc_md58", null ]
      ] ],
      [ "Func", "md_90__message.html#autotoc_md59", [
        [ "func info (kind = 84)", "md_90__message.html#autotoc_md60", null ],
        [ "call (kind = 81)", "md_90__message.html#autotoc_md61", null ],
        [ "call response (kind = 82)", "md_90__message.html#autotoc_md62", null ],
        [ "call result (kind = 83)", "md_90__message.html#autotoc_md63", null ]
      ] ],
      [ "Value", "md_90__message.html#autotoc_md64", [
        [ "value (kind = 0)", "md_90__message.html#autotoc_md65", null ],
        [ "value entry (kind = 20)", "md_90__message.html#autotoc_md66", null ],
        [ "value req (kind = 40)", "md_90__message.html#autotoc_md67", null ],
        [ "value res (kind = 60)", "md_90__message.html#autotoc_md68", null ]
      ] ],
      [ "Text", "md_90__message.html#autotoc_md69", [
        [ "text (kind = 1)", "md_90__message.html#autotoc_md70", null ],
        [ "text entry (kind = 21)", "md_90__message.html#autotoc_md71", null ],
        [ "text req (kind = 41)", "md_90__message.html#autotoc_md72", null ],
        [ "text res (kind = 61)", "md_90__message.html#autotoc_md73", null ]
      ] ],
      [ "View", "md_90__message.html#autotoc_md74", [
        [ "view (kind = 3)", "md_90__message.html#autotoc_md75", null ],
        [ "view entry (kind = 23)", "md_90__message.html#autotoc_md76", null ],
        [ "view req (kind = 43)", "md_90__message.html#autotoc_md77", null ],
        [ "view res (kind = 63)", "md_90__message.html#autotoc_md78", null ]
      ] ],
      [ "Log", "md_90__message.html#autotoc_md79", [
        [ "log (kind = 86)", "md_90__message.html#autotoc_md80", null ],
        [ "log req (kind = 87)", "md_90__message.html#autotoc_md81", null ]
      ] ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classWEBCFACE__NS_1_1Common_1_1Arg.html#a4d739cc5d335052eb9f5b2ca559b81d1",
"classWEBCFACE__NS_1_1Common_1_1ValAdaptor.html#a52b37d81a8646f1b51d40fcb13523209",
"classWEBCFACE__NS_1_1Internal_1_1FuncResultStore.html#ac20cfc93c1339d86ba319285ea35acad",
"classWEBCFACE__NS_1_1Member.html#af9c306505ae358dd2fef77173cebc6c8",
"classWEBCFACE__NS_1_1View.html#a8a744b43949a3939f448facad211e3d2",
"functions_vars_h.html",
"namespaceWEBCFACE__NS_1_1Common.html#acd1dd9e2d5f885ad3f73197cfa6b0185",
"structWEBCFACE__NS_1_1Common_1_1FieldBaseComparable.html#aabdf7f98f95dfc7d64dfb3d7f77dd767",
"structWEBCFACE__NS_1_1Internal_1_1ClientData.html#a516e150f6a16175387fb4ba8b4af4839",
"structWEBCFACE__NS_1_1Message_1_1LogReq.html#ab4fc52453246166f09c4cecd507a38a7",
"structWEBCFACE__NS_1_1Message_1_1View_1_1ViewComponent.html#ab0fbaf4ca7a2787586a4b9fdddd53039"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';