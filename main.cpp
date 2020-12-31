#include <iostream>
#include <fstream>
#include <string>

#include "prase.hpp"

void generate_tag(Node *root, std::string &pre, std::string &post) {
    std::string t;
    switch (root->type) {
    case heading:
        t = std::to_string(root->contents.front()->raw.size());
        pre = "<h" + t +">";
        post = "</h" + t +">";
        break;
    case paragraph:
        pre = "<p>";
        post = "</p>";
        break;
    case quote:
        pre = "<blockquote>";
        post = "</blockquote>";
        break;
    case strong:
        pre = "<strong>";
        post = "</strong>";
        break;
    case italic:
        pre = "<em>";
        post = "</em>";
        break;
    case span:
        pre = "<code>";
        post = "</code>";
        break;
    case marker:
        break;
    case code:
        pre = "<pre><code>";
        post = "</code></pre>";
        break;
    case text:
        break;
    case list:
        pre = "<li>";
        post = "</li>";
        break;
    case ol:
        pre = "<ol>";
        post = "</ol>";
        break;
    case ul:
        pre = "<ul>";
        post = "</ul>";
        break;
    case link:
        t = root->contents.back()->raw;
        pre = "<a href=\"" + t + "\">";
        post = "</a>";
        break;
    case image:
        t = root->contents.back()->raw;
        pre = "<img src=\"" + t + "\" alt=\"" + root->contents.front()->raw + "\">";
        post = "</img>";
        break;
    case hr:
        pre = "<hr />";
        post = "";
        break;
    case inlinemath:
        pre = "\\(";
        post = "\\)";
        break;
    case math:
        pre = "\\[";
        post = "\\]";
        break;
    }
}

std::string dfs(Node *root) {
    std::string pre, post;
    generate_tag(root, pre, post);
    std::string content = root->type == marker ? "" : root->raw;
    for (Node *s : root->contents) {
        if (s) content.append(dfs(s));
    }
    return pre + content + post;
}

int main (int argc, char const *argv[]) {
    std::string s;
    std::ifstream fin(argv[1]);
    // std::ifstream fin("test.md");
    Node* root = new Node();
    Praser praser(root);
    std::string html;
    while (getline(fin, s)) {
        praser.prase_block(s);
    }
    html = dfs(root);
    // 准备要写入的 HTML 文件头尾信息
    std::string head = "<!DOCTYPE html><html><head>\
        <meta charset=\"utf-8\">\
        <title>标题</title>\
        <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/github-markdown-css/4.0.0/github-markdown.min.css\">\
        </head><body><article class=\"markdown-body\">\
        <script src=\"https://polyfill.io/v3/polyfill.min.js?features=es6\"></script>\
        <script id=\"MathJax-script\" async src=\"https://cdn.jsdelivr.net/npm/mathjax@3.0.1/es5/tex-mml-chtml.js\"></script>";
    std::string end = "</article></body></html>";
 
    // 将结果写入到文件
    std::ofstream out;
    out.open("output/index.html");
    // 将转换后的内容构造到 HTML 的 <article></article> 标签内部
    out << head + html + end;
    out.close();
}