#include <iostream>
#include <fstream>
#include <string>

#include "convert.h"

int main (int argc, char const *argv[]) {
    auto html = to_html(argv[1]);
    // std::ifstream fin("test.md");
    // 准备要写入的 HTML 文件头尾信息
    std::string head = "<!DOCTYPE html><html><head>\
        <meta charset=\"utf-8\">\
        <title>标题</title>\
        <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/github-markdown-css/4.0.0/github-markdown.min.css\">\
        </head><body><article class=\"markdown-body\">\
        <script src=\"https://polyfill.io/v3/polyfill.min.js?features=es6\"></script>\
        <script id=\"MathJax-script\" async src=\"https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js\"></script>";
    std::string end = "</article></body></html>";
 
    // 将结果写入到文件
    std::ofstream out;
    out.open("output/index.html");
    // 将转换后的内容构造到 HTML 的 <article></article> 标签内部
    out << head + html + end;
    out.close();
}