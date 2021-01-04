#ifndef CONVERT_H
#define CONVERT_H

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

std::string to_html(std::string filename) {
    std::ifstream fin(filename);
    Node* root = new Node();
    Praser praser(root);
    std::string html, s;
    while (getline(fin, s)) {
        praser.prase_block(s);
    }
    html = dfs(root);
    return html;
}

#endif