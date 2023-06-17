#!/usr/bin/env python3
try:
    import clang.cindex as cindex
except:
    import cindex
import argparse

class Fn:
    def __init__(self, cur, namespace):
        self.valid = True
        self.result_type = cur.result_type.spelling
        self.name = cur.spelling
        if not self.is_valid_type(cur.result_type, next(cur.get_tokens())):
            self.valid = False
        self.namespace = namespace
        self.args = []
        for cur_arg in cur.get_children():
            if cur_arg.kind == cindex.CursorKind.PARM_DECL:
                self.args.append({"name":cur_arg.spelling, "type":cur_arg.type.spelling})
                if not self.is_valid_type(cur_arg.type, next(cur_arg.get_tokens())):
                    self.valid = False
    def is_valid_type(self, ty, token):
        # 認識できない型はintになるがtokenで元の文字列が得られるためそれがintかどうかを確認
        if ty.spelling == "int" and token.spelling != "int":
            print(f"{self.name}: '{token.spelling}'がパースできないため無視します")
            return False
        # using namespace std;とかされて string になっててもstd::stringとして認識されるっぽい
        if ty.spelling in ["void", "int", "double", "bool", "std::string"]:
            return True
        # if re.compile(r"(std::)?u?int_?\w*\d*_t").match(t):
        #     return True
        print(f"{self.name}: 型'{ty.spelling}'には非対応のため無視します")
        return False
    def __str__(self):
        return f"{{fn: {self.result_type} {self.namespace}::{self.name}({self.args});}}"

def parse_namespace(cur_namespace, namespace, target_namespace):
    namespace_match = namespace == target_namespace or namespace.startswith(target_namespace + "::")
    funcs = []
    for cur in cur_namespace.get_children():
        if cur.kind == cindex.CursorKind.FUNCTION_DECL and namespace_match:  # type: ignore
            fn = Fn(cur, namespace)
            if fn.valid:
                funcs.append(fn)
        if cur.kind == cindex.CursorKind.NAMESPACE:
            namespace_next = cur.spelling
            if namespace != "":
                namespace_next = namespace + "::" + namespace_next
            funcs += parse_namespace(cur, namespace_next, target_namespace)
    return funcs

def parse_file(ns, src):
    tu = cindex.TranslationUnit.from_source(
        src,
        args=["-std=c++20"],
        options=cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES
    )
    for d in tu.diagnostics:
        ("clang diagnostics: " + d.format())
    cur_root = tu.cursor
    funcs = parse_namespace(cur_root, "", ns)
    return funcs

def stdout_cpp(root_namespace, funcs, ofs):
    ret = (
        "#include <string>\n"
    )
    for fn in funcs:
        s_args = [f"{a['type']} {a['name']}" for a in fn.args]
        ret += (
            f"namespace {fn.namespace} {{"
            f"{fn.result_type} {fn.name}({', '.join(s_args)});"
            "}\n"
        )
    ret += (
        "#include <webcface/registration.hpp>\n"
        "namespace WebCFace {\n"
        "void addGeneratedFunctions() {\n"
    )
    for fn in funcs:
        full_name = f"{fn.namespace}::{fn.name}"
        full_name_without_shell = full_name[len(root_namespace) + 2: ]
        if fn.result_type == "void":
            s_args = [f"\"{a['name']}\"" for a in fn.args]
            ret += f'WebCFace::addFunctionToRobot("{full_name_without_shell.replace("::", ".")}", {full_name}, {{ {", ".join(s_args)} }});\n'
        elif len(fn.args) == 0:
            ret += f'WebCFace::addFunctionFromRobot("{full_name_without_shell.replace("::", ".")}", {full_name});\n'
        else:
            print(f"{fn.name}: 引数も戻り値もある関数は無視します")
    ret += (
        "}\n"
        "}\n"
    )
    ofs.write(ret)

def main():
    funcs = []
    parser = argparse.ArgumentParser()
    parser.add_argument("--file", nargs="*")
    parser.add_argument("--namespace")
    parser.add_argument("--libclang_path")
    parser.add_argument("--output")
    args = parser.parse_args()
    if args.libclang_path is not None:
        cindex.Config.set_library_file(args.libclang_path)
    ns = args.namespace
    for src in args.file:
        funcs += parse_file(ns, src)
    with open(args.output, "w") as ofs:
        stdout_cpp(ns, funcs, ofs)
    print(f" {len(funcs)} 個の関数が見つかりました\n")

if __name__ == '__main__':
    main()
