#ifndef   __TOY_LEXER_H__
#define   __TOY_LEXER_H__

#include "llvm/ADT/StringRef.h"

#include <string>
#include <memory>


namespace toy
{

/// 记录代码中的位置信息，便于调试和错误报告
struct Location
{
    std::shared_ptr<std::string> file; // 文件名
    int line;                          // 行号
    int col;                           // 列号
};


/// 定义词法分析器返回的token类型
enum Token : int {
    tok_semicolon = ';',        // 分号
    tok_parenthese_open = '(',   // 左括号
    tok_parenthese_close = ')', // 右括号
    tok_bracket_open = '{',     // 左大括号
    tok_bracket_close = '}',    // 右大括号
    tok_sbracket_open = '[',    // 左中括号
    tok_sbracket_close = ']',   // 右中括号

    tok_eof = -1,               // 文件结束

    tok_return = -2,             // return
    tok_var = -3,                // var
    tok_def = -4,                // def


    tok_identifier = -5,         // 标识符
    tok_number = -6,             // 数字
};

/// 词法分析的抽象基类，提供解析器所需的接口
/// 它逐个读取输入流中的 Token, 并记录文件中的位置信息，便于调试
/// 它依赖子类实现的‵readnextLine()‵方法，用于标准输入或内存映射文件中读取下一行
class Lexer{
public:
    /// 构造函数：根据文件名创建词法分析器实例
    Lexer(std::string filename) :
        lastLocation({std::make_shared<std::string>(filename), 0, 0}) {}

    virtual ~Lexer() = default;

    /// 查看当前Token
    Token getCurToken() { return curTok; }

    /// 移动到下一个Token并返回
    Token getNextToken() { return curTok = getTok(); }

    /// 移动到下一个Token,并断言当前Token符合预期
    void consume(Token tok) {
        assert(tok == curTok && " consume Token mismatch expectation");
        getNextToken();
    }

    /// 返回当前标识符（要求当前Token为标识符）
    llvm::StringRef getId(){
        assert(curTok == tok_identifier);
        return identifierStr;
    }

    ///  Return the current number (prereq: getCurToken() == tok_number)
    double getValue() {
        assert(curTok == tok_number);
        return numVal;
    }

    /// 返回当前数字 （要求当前Token为数字）
    double getNumber(){
        assert(curTok == tok_number);
        return numVal;
    }

    /// 返回当前Token的位置信息
    Location getLastLocation(){
        return lastLocation;
    }

    /// 返回当前行号
    int getLine(){
        return curLine;
    }

    /// 返回当前列号
    int getCol(){
        return  curCol;
    }

private:
    Location lastLocation;     // 当前Token的位置信息

    Token curTok = tok_eof;    // 当前Token

    std::string identifierStr; // 当前Token 时标识符的值

    double numVal = 0;         // 当前Token 时数字的值

    int curLine = 0;           // 当前行号

    int curCol = 0;            // 当前列号


private:

    /// 由子类实现：获取下一行内容。返回空字符串表示文件结束
    virtual llvm::StringRef readNextLine() = 0;

    /// 从输入流中返回下一个Token
    Token getTok() {
        while (isspace(lastChar))
            lastChar = Token(getNextChar());

            // 记录当前Token的位置信息
            lastLocation.line = curLine;
            lastLocation.col = curCol;

            // 如果当前字符是字母[a-zA-Z][a-zA-Z0-9_]*，则从输入流中返回下一个标识符
            if(isalpha(lastChar)) {
                identifierStr = (char)lastChar;
                while (isalnum(lastChar = Token(getNextChar())) || lastChar == '_')
                    identifierStr += (char)lastChar;
                if (identifierStr == "var")
                    return tok_var;
                if (identifierStr == "def")
                    return tok_def;
                if (identifierStr == "return")
                    return tok_return;

                return tok_identifier;
            }

            // 如果当前字符是数字[0-9.]+，则从输入流中返回下一个数字
            if(isdigit(lastChar) || lastChar == '.') {
                std::string numStr;
                do
                {
                    numStr += lastChar;
                    lastChar = Token(getNextChar());
                } while (isdigit(lastChar) || lastChar == '.');

                numVal = strtod(numStr.c_str(), nullptr);
                return tok_number;
            }

            if(lastChar == '#') {
                // 判断输入流中是否有注释
                do
                {
                    lastChar = Token(getNextChar());
                } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

                if (lastChar != EOF)
                {
                    return getTok();
                }
            }

            // 如果当前字符是 EOF，则返回 EOF
            if (lastChar == EOF)
            {
                return tok_eof;
            }

            // 返回当前字符
            Token thisChar = Token(lastChar);
            lastChar = Token(getNextChar());
            return thisChar;
    }

    /// 从输入流中返回下一个字符
    int getNextChar() {
        if (curLineBuffer.empty())
            return EOF;
        // 移动一个字符
        ++curCol;
        // 从 curLineBuffer 的前端获取第一个字符，并将其赋值给变量 nextChar
        auto nextChar = curLineBuffer.front();
        // 将 curLineBuffer 更新为移除第一个字符后的新缓冲区
        curLineBuffer = curLineBuffer.drop_front();

        // 一行结束 更新
        if(curLineBuffer.empty())
            curLineBuffer = readNextLine();
        if (nextChar == '\n')
        {
            ++curLine;
            curCol = 0;
        }
        return nextChar;
    }

private :
    llvm::StringRef curLineBuffer = "\n"; // 当前行缓冲区

    Token lastChar = Token(' ');          // 上一个字符
};

///  在内存缓冲区上操作的词法分析器实现
class LexerBuffer final : public Lexer{
public:
    LexerBuffer(const char* begin,const char *end, std::string finename) :
         Lexer(std::move(finename)), current(begin), end(end) {}

private:
    const char *current, *end;  // 当前位置和结束位置
    /// 逐行提供给词法分析器，到达缓冲区末尾时返回1
    llvm::StringRef readNextLine() override {
        auto *begin = current;
        while (current != end && *current != '\n' && *current)
            ++current;
        if (current <= end && *current)
            ++current;
        llvm::StringRef result{begin, static_cast<size_t>(current - begin)};
        return result;
    }
};


} // namespace toy

#endif  // __TOY_LEXER_H__