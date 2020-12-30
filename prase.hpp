#ifndef PRASE_H
#define PRASE_H

#include <string>
#include <stack>
#include <list>
#include <utility>

#include <iostream>

enum NodeType {
    heading,
    paragraph,
    quote,
    strong,
    italic,
    span,
    marker,
    code,
    text,
    list,
    ul,
    ol,
    link,
    image,
    hr
};

class Node {
public:
    NodeType type;
    std::string raw;
    std::list<Node*> contents;

    Node() {
    }

    Node(const std::string &s, NodeType type) : raw(s), type(type) {
    }

};


class Context {
private:
    unsigned status;
};

class Praser {
private:
    Context status;
    bool is_code_block = false;
    bool is_list_block = false;
    Node *cross_line_p = nullptr;
public:
    Node* solve_line(Node *p, const std::string &content, unsigned i=0) {
        while(content[i] == ' ') i++;
        std::stack<std::pair<unsigned, NodeType>> op;
        NodeType cur_type;
        while (i < content.size()) {
            switch (content[i]) {
            case '*': 
            case '_': 
                if (content[i + 1] == '*' or content[i + 1] == '_') {
                    i++;
                    cur_type = NodeType::strong;
                } else {
                    cur_type = NodeType::italic;
                }
                if (op.empty() or op.top().second != cur_type) {
                    if (not op.empty() and op.top().second == text) {
                        Node *text_child = new Node(content.substr(op.top().first, i - op.top().first - (cur_type == strong)), text);
                        p->contents.push_back(text_child);
                        op.pop();
                    }
                    // todo: check content[i+1] != '*'
                    op.emplace(std::make_pair(i + 1, cur_type));
                } else {
                    unsigned pre_i = op.top().first;
                    op.pop();
                    Node *new_child = new Node(content.substr(pre_i, i - pre_i - (cur_type == strong)), cur_type);
                    p->contents.push_back(new_child);
                }
                break;
            case '`': 
                cur_type = span;
                if (op.empty() or op.top().second != cur_type) {
                    if (not op.empty() and op.top().second == text) {
                        Node *text_child = new Node(content.substr(op.top().first, i - op.top().first - (cur_type == strong)), text);
                        p->contents.push_back(text_child);
                        op.pop();
                    }
                    // todo: check content[i+1] != '*'
                    op.emplace(std::make_pair(i + 1, cur_type));
                } else {
                    unsigned pre_i = op.top().first;
                    op.pop();
                    Node *new_child = new Node(content.substr(pre_i, i - pre_i), cur_type);
                    p->contents.push_back(new_child);
                }
                break;
            case '[': 
                if (is_link(content, i)) {
                    if (not op.empty() and op.top().second == text) {
                        Node *text_child = new Node(content.substr(op.top().first, i - op.top().first - (cur_type == strong)), text);
                        p->contents.push_back(text_child);
                        op.pop();
                    }
                    auto l = solve_link(content, i);
                    p->contents.push_back(l);
                }
                goto text_process;
            case '!': 
                if (is_image(content, i)) {
                    if (not op.empty() and op.top().second == text) {
                        Node *text_child = new Node(content.substr(op.top().first, i - op.top().first - (cur_type == strong)), text);
                        p->contents.push_back(text_child);
                        op.pop();
                    }
                    auto l = solve_image(content, i);
                    p->contents.push_back(l);
                }
                goto text_process;
            default: 
  text_process: cur_type = text;
                if (op.empty()) {
                    // todo: check content[i+1] != '*'
                    op.emplace(std::make_pair(i, cur_type));
                }
                break;
            }
            i++;
        }
        if (not op.empty() and op.top().second == text) {
            Node *text_child = new Node(content.substr(op.top().first, i - op.top().first - (cur_type == strong)), text);
            p->contents.push_back(text_child);
        }
        return nullptr;
    }

    bool is_link(const std::string &content, unsigned i) {
        if (content[i] == '[') {
            for (unsigned j = i + 1; j < content.length(); j++) {
                if (content.substr(j, 2) == "](") {
                    return true;
                }
            }
        }
        return false;
    }

    bool is_image(const std::string &content, unsigned i) {
        return content[i] == '!' and is_link(content, i + 1);
    }

    Node* solve_link(const std::string &content, unsigned &i) {
        Node *p = new Node();
        p->type = link;
        i = i + 1;
        unsigned j = i;
        bool ok = false;
        while (j < content.length()) {
            if (content.substr(j, 2) == "](") {
                // [as]()
                p->contents.push_back(new Node(content.substr(i, j - i), text));
                ok = true;
                break;
            }
            j++;
        }
        if (not ok) return nullptr;
        ok = false;
        j = j + 2;
        i = j;
        while (j < content.length()) {
            if (content[j] == ')') {
                p->contents.push_back(new Node(content.substr(i, j - i), marker));
                ok = true;
                break;
            }
            j++;
        }
        i = j + 1;
        if (not ok) return nullptr;
        return p;
    }

    Node* solve_image(const std::string &content, unsigned &i) {
        Node *p = new Node();
        p->type = image;
        i = i + 2;
        unsigned j = i;
        bool ok = false;
        while (j < content.length()) {
            if (content.substr(j, 2) == "](") {
                p->contents.push_back(new Node(content.substr(i, j - i), marker));
                ok = true;
                break;
            }
            j++;
        }
        if (not ok) return nullptr;
        ok = false;
        j = j + 2;
        i = j;
        while (j < content.length()) {
            if (content[j] == ')') {
                p->contents.push_back(new Node(content.substr(i, j - i), marker));
                ok = true;
                break;
            }
            j++;
        }
        i = j + 1;
        if (not ok) return nullptr;
        return p;
    }


    bool is_ordered_list(const std::string &content, unsigned i) {
        unsigned cnt = 0;
        while (isdigit(content[i + cnt])) cnt++;
        if (cnt != 0 and 
            i + cnt + 1 < content.length() and 
            content[i + cnt + 1] == ' ' and 
            (content[i + cnt] == '.' or content[i + cnt] == ')')) {
                return true;
        }
        return false;
    }

    bool is_unordered_list(const std::string &content, unsigned i) {
        return i + 1 < content.length() and 
        content.substr(i, 2) == "* " or 
        content.substr(i, 2) == "- " or
        content.substr(i, 2) == "+ ";
    }
    
    Node* prase(const std::string &content, unsigned i=0) {
        while(content[i] == ' ') i++;
        if (is_list_block) {
            if (not is_ordered_list(content, i) and not is_unordered_list(content, i)) {
                auto t = cross_line_p;
                is_list_block = false;
                cross_line_p = nullptr;
                return t;
            }
        }
        Node *p = (is_code_block or is_list_block) ? cross_line_p : new Node();

        // codeblock
        if (content.substr(i, 3) == "$$$" or content.substr(i, 3) == "```") {
            is_code_block = !is_code_block;
            if (is_code_block) {
                p->type = code;
                p->contents.push_back(new Node(content.substr(i + 3, content.length() - 3), NodeType::marker));
                cross_line_p = p;
                return nullptr;
            } else {
                cross_line_p = nullptr;
                return p;
            }
        } else if (is_code_block) {
            p->contents.push_back(new Node(content + "\n", NodeType::text));
            return nullptr;
        }

        if (content.substr(i, 3) == "---") {
            Node *p = new Node();
            p->type = hr;
            return p;
        }

        if (content[i] == '\0') {
            return nullptr;
        }

        // listblock
        if (is_unordered_list(content, i)) {
            if (not is_list_block) {
                is_list_block = true;
                p->type = ul;
                cross_line_p = p;
            }
        } else if (is_ordered_list(content, i)) {
            if (not is_list_block) {
                is_list_block = true;
                p->type = ol;
                cross_line_p = p;
            }
        }

        unsigned j = i, cnt;
        // split
        switch (content[i]) {
        case '#': 
            while (content[j] == '#') j++;
            p->type = heading;
            p->contents.push_back(new Node(content.substr(i, j - i), NodeType::marker));
            solve_line(p, content, j);
            break;
        case '>': 
            while (content[j] == '>') j++;
            p->type = quote;
            p->contents.push_back(new Node(content.substr(i, j - i), NodeType::marker));
            solve_line(p, content, j);
            break;
        case '*':
        case '-':
        case '+':
            if (content[j + 1] == ' ') {
                j++;
                // p->contents.push_back(new Node(content.substr(i, j - i), NodeType::marker));
                Node *plist = new Node();
                plist->type = list;
                solve_line(plist, content, j);
                p->contents.push_back(plist);
                return nullptr;
            }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
            cnt = 0;
            while (isdigit(content[i + cnt])) cnt++;
            if ((content[i + cnt] == ')' or content[i + cnt] == '.') and content[i + cnt + 1] == ' ') {
                j = i + cnt + 1;
                Node *plist = new Node();
                plist->type = list;
                solve_line(plist, content, j);
                p->contents.push_back(plist);
                return nullptr;
            }
        default:
            p->type = paragraph;
            solve_line(p, content, j);
            break;
        }
        return p;
    }
};

#endif