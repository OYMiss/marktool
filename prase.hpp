#ifndef PRASE_H
#define PRASE_H

#include <string>
#include <stack>
#include <list>
#include <utility>

#include <iostream>

enum NodeType {
    text,
    code,
    inlinemath,
    math,
    ul,
    ol,
    paragraph,
    heading,
    quote,
    strong,
    italic,
    span,
    list,
    link,
    image,
    hr,
    marker,
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

enum Status {
    textstatus,
    codestatus,
    mathstatus,
    uliststatus,
    oliststatus,
};

class Praser {
private:
    Status status = textstatus;
    Node *cross_line_p = nullptr;
    Node *root;

    // function for block check
    bool is_ordered_list_begin(const std::string &content) {
        unsigned cnt = 0;
        while (isdigit(content[cnt])) cnt++;
        if (cnt != 0 and 
            cnt + 1 < content.length() and 
            content[cnt + 1] == ' ' and 
            (content[cnt] == '.' or content[cnt] == ')')) {
                return true;
        }
        return false;
    }

    bool is_unordered_list_begin(const std::string &content) {
        return 0 + 1 < content.length() and 
        content.substr(0, 2) == "* " or 
        content.substr(0, 2) == "- " or
        content.substr(0, 2) == "+ ";
    }

    bool is_blankline_end(const std::string &content) {
        int i = 0;
        while (i < content.length() and content[i] == ' ' or content[i] == '\t') i++;
        return content[i] == '\0';
    }

    bool is_list_end(const std::string &content) {
        return is_blankline_end(content);
    }

    bool is_code_begin(const std::string &content) {
        return content.substr(0, 3) == "```";
    }

    bool is_math_begin(const std::string &content) {
        return content.substr(0, 2) == "$$";
    }

    bool is_code_end(const std::string &content) {
        return content.substr(0, 3) == "```";
    }

    bool is_math_end(const std::string &content) {
        return content.substr(0, 2) == "$$";
    }

    // function for line check
    bool check_heading_and_move(const std::string &content, unsigned &i) {
        if (content[i] == '#') {
            while (i < content.length() and content[i] == '#') i++;
            while (i < content.length() and content[i] == ' ') i++;
            return true;
        }
        return false;
    }

    bool check_quote_and_move(const std::string &content, unsigned &i) {
        if (content[i] == '>') {
            if (i < content.length() and content[i] == '>') i++;
            while (i < content.length() and content[i] == ' ') i++;
            return true;
        }
        return false;
    }

    bool check_ulist_and_move(const std::string &content, unsigned &i) {
        if (is_unordered_list_begin(content)) {
            i += 2;
            return true;
        } else {
            return false;
        }
    }

    bool check_olist_and_move(const std::string &content, unsigned &i) {
        if (is_ordered_list_begin(content)) {
            while (i < content.length() and isdigit(content[i])) i++;
            i += 2;
            return true;
        } else {
            return false;
        }
    }

    bool check_hr_and_move(const std::string &content, unsigned &i) {
        if (content.substr(i, 3) == "---") {
            return true;
        } else {
            return false;
        }
    }

    // function for paragraph check
    bool check_italic_and_move(const std::string &content, unsigned &i, unsigned &j, unsigned &nexti) {
        if (content[i] == '*' or content[i] == '_') {
            j = i + 1;
            while (j < content.length() and content[j] != content[i]) j++;
            i = i + 1;
            nexti = j + 1;
            j = j - i;
            return true;
        }
        return false;
    }

    bool check_strong_and_move(const std::string &content, unsigned &i, unsigned &j, unsigned &nexti) {
        if (i + 1 < content.length() and content[i] == '*' and content[i + 1] == '*') {
            i = i + 2;
            j = i + 2;
            while (j < content.length() and content[j] != '*') j++;
            nexti = j + 2;
            j = j - i;
            return true;
        }
        return false;
    }

    bool check_span_and_move(const std::string &content, unsigned &i, unsigned &j, unsigned &nexti) {
        if (content[i] == '`') {
            i = i + 1;
            j = i + 1;
            while (j < content.length() and content[j] != '`') j++;
            nexti = j + 1;
            j = j - i;
            return true;
        }
        return false;
    }

    bool check_inlinemath_and_move(const std::string &content, unsigned &i, unsigned &j, unsigned &nexti) {
        if (content[i] == '$') {
            i = i + 1;
            j = i + 1;
            while (j < content.length() and content[j] != '$') j++;
            nexti = j + 1;
            j = j - i;
            return true;
        }
        return false;
    }

    bool check_image_and_move(const std::string &content, unsigned &i, unsigned &j, unsigned &k, unsigned &l, unsigned &nexti) {
        if (content[i] == '!') {
            unsigned ii = i + 1;
            if (check_link_and_move(content, ii, j, k, l, nexti)) {
                i = ii;
                return true;
            }
        }
        return false;
    }

    bool check_link_and_move(const std::string &content, unsigned &i, unsigned &j, unsigned &k, unsigned &l, unsigned &nexti) {
        if (content[i] == '[') {
            bool ok = false;
            for (j = i + 1; j < content.length() - 1; j++) {
                if (content.substr(j, 2) == "](") {
                    ok = true;
                    break;
                } else if (content.substr(j, 2) == "] ") {
                    return false;
                }
            }

            if (not ok) return false;
            k = j + 2;
            for (l = k; l < content.length(); l++) {
                if (content[l] == ')') {
                    nexti = l + 1;
                    i++;
                    j = j - i;
                    l = l - k;
                    return true;
                }
            }
        }
        return false;
    }

    void clear_text_buff(std::string &buffer, Node *p) {
        if (not buffer.empty()) {
            p->contents.push_back(new Node(buffer, text));
            buffer.clear();
        }
    }

    Node* prase_line(const std::string &content) {
        unsigned i = 0;
        Node *p = nullptr;
        if (check_heading_and_move(content, i)) {
            p = new Node();
            p->type = heading;
            p->contents.push_back(new Node(content.substr(0, i - 1), NodeType::marker));
            p->contents.push_back(prase_paragraph(content, i, false));
        } else if (check_quote_and_move(content, i)) {
            p = new Node();
            p->type = quote;
            p->contents.push_back(new Node(content.substr(0, i - 1), NodeType::marker));
            p->contents.push_back(prase_paragraph(content, i, true));
        } else if (check_ulist_and_move(content, i) or check_olist_and_move(content, i)) {
            p = new Node();
            p->type = list;
            p->contents.push_back(prase_paragraph(content, i, false));
        } else if (check_hr_and_move(content, i)) {
            p = new Node();
            p->type = hr;
        } else {
            p = prase_paragraph(content, i);
        }
        return p;
    }

    Node* prase_paragraph(const std::string &content, unsigned i, bool is_paragraph=true) {
        while (i < content.length() and content[i] == ' ') i++;
        if (content[i] == '\0') return nullptr;
        Node *p = new Node();
        p->type = is_paragraph ? paragraph : text;
        std::string buffer;
        unsigned j, k, l, nexti;
        while (i < content.length()) {
            if (check_strong_and_move(content, i, j, nexti)) {
                clear_text_buff(buffer, p);
                p->contents.push_back(new Node(content.substr(i, j), strong));
                i = nexti;
            } else if (check_italic_and_move(content, i, j, nexti)) {
                clear_text_buff(buffer, p);
                p->contents.push_back(new Node(content.substr(i, j), italic));
                i = nexti;
            } else if (check_span_and_move(content, i, j, nexti)) {
                clear_text_buff(buffer, p);
                p->contents.push_back(new Node(content.substr(i, j), span));
                i = nexti;
            } else if (check_inlinemath_and_move(content, i, j, nexti)) {
                clear_text_buff(buffer, p);
                p->contents.push_back(new Node(content.substr(i, j), inlinemath));
                i = nexti;
            } else if (check_link_and_move(content, i, j, k, l, nexti)) {
                clear_text_buff(buffer, p);
                Node *plink = new Node();
                plink->type = link;
                plink->contents.push_back(new Node(content.substr(i, j), text));
                plink->contents.push_back(new Node(content.substr(k, l), marker));
                p->contents.push_back(plink);
                i = nexti;
            } else if (check_image_and_move(content, i, j, k, l, nexti)) {
                clear_text_buff(buffer, p);
                Node *pimg = new Node();
                pimg->type = image;
                pimg->contents.push_back(new Node(content.substr(i, j), marker));
                pimg->contents.push_back(new Node(content.substr(k, l), marker));
                p->contents.push_back(pimg);
                i = nexti;
            } else {
                buffer.push_back(content[i]);
                i++;
            }
        }
        clear_text_buff(buffer, p);
        return p;
    }

public:
    Praser(Node *root) : root(root) {
    }

    void prase_block(const std::string &content) {
        // check end
        switch (status) {
        case codestatus:
            if (is_code_end(content)) {
                status = textstatus;
                root->contents.push_back(cross_line_p);
                cross_line_p = nullptr;
            } else {
                cross_line_p->contents.push_back(new Node(content + "\n", NodeType::text));
            }
            break;
        case mathstatus:
            if (is_math_end(content)) {
                status = textstatus;
                root->contents.push_back(cross_line_p);
                cross_line_p = nullptr;
            } else {
                cross_line_p->contents.push_back(new Node(content + "\n", NodeType::text));
            }
            break;
        case uliststatus:
        case oliststatus:
            if (is_list_end(content)) {
                status = textstatus;
                root->contents.push_back(cross_line_p);
                cross_line_p = nullptr;
            } else {
                assert(cross_line_p != nullptr);
                cross_line_p->contents.push_back(prase_line(content));
            }
            break;
        default:
            // check begin
            if (is_code_begin(content)) {
                status = codestatus;
                cross_line_p = new Node();
                cross_line_p->type = code;
                cross_line_p->contents.push_back(new Node(content.substr(3, content.length() - 3), marker));
            } else if (is_math_begin(content)) {
                status = mathstatus;
                cross_line_p = new Node();
                cross_line_p->type = math;
                cross_line_p->contents.push_back(new Node(content.substr(2, content.length() - 2), marker));
            } else if (is_unordered_list_begin(content)) {
                status = uliststatus;
                cross_line_p = new Node();
                cross_line_p->type = ul;
                cross_line_p->contents.push_back(prase_line(content));
            } else if (is_ordered_list_begin(content)) {
                status = oliststatus;
                cross_line_p = new Node();
                cross_line_p->type = ol;
                cross_line_p->contents.push_back(prase_line(content));
            } else {
                auto newnode = prase_line(content);
                if (newnode) root->contents.push_back(newnode);
            }
        }
    }
};

#endif