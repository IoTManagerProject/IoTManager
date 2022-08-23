#pragma once
#include "Global.h"
#include "classes/IoTItem.h"
#include <map>

class ExprAST {
   public:
    virtual ~ExprAST();
    virtual IoTValue *exec();
    virtual int setValue(IoTValue *val);  // ret 0 - установка значения не поддерживается наследником
    virtual bool hasEventIdName(String eventIdName);
};

class IoTScenario {
    //===----------------------------------------------------------------------===//
    // Lexer (Лексический анализатор)
    //===----------------------------------------------------------------------===//

    String IdentifierStr;  // Заполняется, если tok_identifier
    float NumVal;          // Заполняется, если tok_number
    char LastChar = ' ';

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

    /// BinopPrecedence - Содержит приоритеты для бинарных операторов
    std::map<signed char, int> BinopPrecedence;

    /// GetTokPrecedence - Возвращает приоритет текущего бинарного оператора.
    int GetTokPrecedence();

    /// Error* - Это небольшие вспомогательные функции для обработки ошибок.
    ExprAST *Error(const char *Str);

    /// identifierexpr
    ///   ::= identifier
    ///   ::= identifier '(' expression* ')'
    ExprAST *ParseIdentifierExpr(String *IDNames);

    /// numberexpr ::= number
    ExprAST *ParseNumberExpr();

    /// parenexpr ::= '(' expression ')'
    ExprAST *ParseParenExpr();

    /// bracketsexpr ::= '{' expression '}'
    ExprAST *ParseBracketsExpr();

    /// quotesexpr ::= '"' expression '"'
    ExprAST *ParseQuotesExpr();

    /// ifexpr ::= 'if' expression 'then' expression 'else' expression
    ExprAST *ParseIfExpr(String *IDNames);

    /// primary
    ///   ::= identifierexpr
    ///   ::= numberexpr
    ///   ::= parenexpr
    ExprAST *ParsePrimary(String *IDNames);

    /// binoprhs
    ///   ::= ('+' primary)*
    ExprAST *ParseBinOpRHS(int ExprPrec, ExprAST *LHS, String *IDNames);

    /// expression
    ///   ::= primary binoprhs
    ///
    ExprAST *ParseExpression(String *IDNames);

    std::vector<ExprAST *> ScenarioElements;  // корневые элементы дерава

    String *strFromFile;
    char getLastChar();
    int strIterator = 0;

    void clearScenarioElements();

   public:
    void loadScenario(String fileName);
    void ExecScenario(String eventIdName);

    IoTScenario();
    ~IoTScenario();
};
