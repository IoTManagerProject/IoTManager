#pragma once
#include "Global.h"
#include "classes/IoTItem.h"
#include <map>

class ExprAST {
   public:
    virtual ~ExprAST();
    virtual IoTValue *exec();
    virtual int setValue(IoTValue *val, bool generateEvent);  // ret 0 - установка значения не поддерживается наследником
    virtual bool hasEventIdName(const String& eventIdName);
};

class IoTScenario {
    int mode = 0;   // режим работы: 
                    // 0 - весь сценарий хранится в FS, читаем посимвольно из файла. Максимальная экономии памяти, но медленно
                    // 1 - весь сценарий подгружается в String, читаем посимвольно из строки
                    // 2 - сценарий конвертируется в структуру классов за один проход и выполняется только в памяти. Максимальная
                    // скорость, но на 8266 при средних сценариях не хватает памяти

    String strFromFile;
    int charCount;
    
    //===----------------------------------------------------------------------===//
    // Lexer (Лексический анализатор)
    //===----------------------------------------------------------------------===//

    String IdentifierStr;  // Заполняется, если tok_identifier
    String NumStr = "";          // Заполняется, если tok_number
    int LastChar;
    int curLine;

    /// gettok - Возвращает следующий токен из стандартного потока ввода.
    int gettok();

    //===----------------------------------------------------------------------===//
    // Parser (Парсер или Синтаксический Анализатор)
    //===----------------------------------------------------------------------===//

    /// CurTok/getNextToken - Предоставляет простой буфер токенов. CurTok - это текущий
    /// токен, просматриваемый парсером. getNextToken получает следующий токен от
    /// лексического анализатора и обновляет CurTok.
    int CurTok;
    int getNextToken();
    String IDNames;     // накопитель встречающихся идентификаторов в условии

    /// BinopPrecedence - Содержит приоритеты для бинарных операторов
    std::map<signed char, int> BinopPrecedence;

    /// GetTokPrecedence - Возвращает приоритет текущего бинарного оператора.
    int GetTokPrecedence();

    /// Error* - Это небольшие вспомогательные функции для обработки ошибок.
    ExprAST *Error(const String& Str);

    /// identifierexpr
    ///   ::= identifier
    ///   ::= identifier '(' expression* ')'
    ExprAST *ParseIdentifierExpr(String *IDNames, bool callFromCondition);

    /// numberexpr ::= number
    ExprAST *ParseNumberExpr();

    /// parenexpr ::= '(' expression ')'
    ExprAST *ParseParenExpr(String *IDNames, bool callFromCondition);

    /// bracketsexpr ::= '{' expression '}'
    ExprAST *ParseBracketsExpr(String *IDNames, bool callFromCondition);

    /// quotesexpr ::= '"' expression '"'
    ExprAST *ParseQuotesExpr();

    /// ifexpr ::= 'if' expression 'then' expression 'else' expression
    ExprAST *ParseIfExpr(String *IDNames);

    /// primary
    ///   ::= identifierexpr
    ///   ::= numberexpr
    ///   ::= parenexpr
    ExprAST *ParsePrimary(String *IDNames, bool callFromCondition);

    /// binoprhs
    ///   ::= ('+' primary)*
    ExprAST *ParseBinOpRHS(int ExprPrec, ExprAST *LHS, String *IDNames, bool callFromCondition);

    /// expression
    ///   ::= primary binoprhs
    ///
    ExprAST *ParseExpression(String *IDNames, bool callFromCondition);

    int getLastChar();
    fs::File file;

   public:
    void loadScenario(String fileName);
    void exec(const String& eventIdName);

    IoTScenario();
    ~IoTScenario();
};
