#include <fstream>

#pragma once
#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTScenario.h"
#include "utils/FileUtils.h"


bool isIotScenException;  // признак исключения и попытки прекратить выполнение сценария заранее


// Лексический анализатор возвращает токены [0-255], если это неизвестны, 
// иначе одну из известных единиц кода
enum Token {
  tok_eof = -1,

  // операнды (первичные выражения: идентификаторы, числа)
  tok_identifier = -4, tok_number = -5, tok_string = -3,

  // двухсимвольные операторы бинарных операций
  tok_equal = -2, tok_notequal = -9, tok_lesseq = -10, tok_greateq = -11,  

  // управление
  tok_if = -6, tok_then = -7, tok_else = -8
};

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (Абстрактное Синтаксическое Дерево или Дерево Парсинга)
//===----------------------------------------------------------------------===//

/// ExprAST - Базовый класс для всех узлов выражений.
ExprAST::~ExprAST() {}
IoTValue* ExprAST::exec() {return nullptr;}
int ExprAST::setValue(IoTValue *val) {return 0;}  // 0 - установка значения не поддерживается наследником
bool ExprAST::hasEventIdName(String eventIdName) {return false;}  // по умолчанию все узлы не связаны с ИД события, для которого выполняется сценарий
//struct IoTValue zeroIotVal;
        
/// NumberExprAST - Класс узла выражения для числовых литералов (Например, "1.0").
class NumberExprAST : public ExprAST {
  IoTValue Val;
public:
  NumberExprAST(float val) { Val.valD = val;}

  IoTValue* exec() {
    if (isIotScenException) return nullptr;
    //Serial.printf("Call from  NumberExprAST: %f\n", Val.valD);
    return &Val;
  }
};

/// StringExprAST - Класс узла выражения для строковых литералов (Например, "Example with spaces and quotes").
class StringExprAST : public ExprAST {
  IoTValue Val;
public:
  StringExprAST(String val) { Val.isDecimal = false; Val.valS = val;}

  IoTValue* exec() {
    if (isIotScenException) return nullptr;
    //Serial.printf("Call from  StringExprAST: %s\n", Val.valS.c_str());
    return &Val;
  }
};

/// VariableExprAST - Класс узла выражения для переменных (например, "a").
class VariableExprAST : public ExprAST {
  String Name;
  IoTItem* Item;  // ссылка на объект модуля (прямой доступ к идентификатору указанному в сценарии), если получилось найти модуль по ID
  bool ItemIsLocal = false;

public:
  VariableExprAST(const String &name, IoTItem* item) : Name(name), Item(item) {
    if (item) ItemIsLocal = item->iAmLocal;
  }

  int setValue(IoTValue *val) {
    if (!ItemIsLocal) Item = findIoTItem(Name);
    if (Item) Item->setValue(*val);
    else return 0;

    return 1;
  }

  IoTValue* exec() {
    if (isIotScenException) return nullptr;
    if (!ItemIsLocal) Item = findIoTItem(Name);
    if (Item) {
      //if (Item->value.isDecimal) 
      //  Serial.printf("Call from  VariableExprAST: %s = %f\n", Name.c_str(), Item->value.valD);
      //else Serial.printf("Call from  VariableExprAST: %s = %s\n", Name.c_str(), Item->value.valS.c_str());
      return &(Item->value);
    }

    return nullptr; // Item не найден.
  }
};

/// BinaryExprAST - Класс узла выражения для бинарных операторов.
class BinaryExprAST : public ExprAST {
  signed char Op;
  ExprAST *LHS, *RHS;
  IoTValue val;
  String lhsStr, rhsStr;

public:
  BinaryExprAST(signed char op, ExprAST *lhs, ExprAST *rhs) 
    : Op(op), LHS(lhs), RHS(rhs) {}

  ~BinaryExprAST() {
    if (LHS) delete LHS;
    if (RHS) delete RHS;
    //Serial.printf("Call from  BinaryExprAST delete\n");
  }

  IoTValue* exec(){
    if (isIotScenException) return nullptr;
    String printStr = "";

    if (Op == tok_equal) printStr = "==";   
    else if (Op == tok_notequal) printStr = "!=";
    else if (Op == tok_lesseq) printStr = "<=";
    else if (Op == tok_greateq) printStr = ">=";
    else printStr = printStr + (char)Op;

    //Serial.printf("Call from  BinaryExprAST: %s\n", printStr.c_str());
    
    if (RHS == nullptr || LHS == nullptr) return nullptr;

    IoTValue* rhs = RHS->exec();  // получаем значение правого операнда для возможного использования в операции присваивания

    if (Op == '=' && LHS->setValue(rhs)) {  // если установка значения не поддерживается, т.е. слева не переменная, то работаем по другим комбинациям далее
      return rhs;                           // иначе возвращаем присвоенное значение справа
    }

    IoTValue* lhs = LHS->exec();  // если присваивания не произошло, значит операция иная и необходимо значение левого операнда
    
    if (lhs != nullptr && rhs !=nullptr) {
      if (lhs->isDecimal && rhs->isDecimal) {
        switch (Op) {
          case '>':
            val.valD = lhs->valD > rhs->valD;
          break;
          case '<':
            val.valD = lhs->valD < rhs->valD;
          break;
          case tok_lesseq:
            val.valD = lhs->valD <= rhs->valD;
          break;
          case tok_greateq:
            val.valD = lhs->valD >= rhs->valD;
          break;
          case tok_equal:
            val.valD = lhs->valD == rhs->valD;
          break;
          case tok_notequal:
            val.valD = lhs->valD != rhs->valD;
          break;

          case '+':
            val.valD = lhs->valD + rhs->valD;
          break;
          case '-':
            val.valD = lhs->valD - rhs->valD;
          break;
          case '*':
            val.valD = lhs->valD * rhs->valD;
          break;
          case '/':
            if (rhs->valD != 0) val.valD = lhs->valD / rhs->valD;
            else val.valD = 3.4E+38;
          break;

          case '|':
            val.valD = lhs->valD || rhs->valD;
          break;
          case '&':
            val.valD = lhs->valD && rhs->valD;
          break;

          default:
            break;
        }
        return &val;
      }

      if (!lhs->isDecimal || !rhs->isDecimal) {
        if (lhs->isDecimal) lhsStr = lhs->valD; else lhsStr = lhs->valS;
        if (rhs->isDecimal) rhsStr = rhs->valD; else rhsStr = rhs->valS;
        switch (Op) {
          case tok_equal:
            val.valD = compStr(lhsStr, rhsStr);
          break;

          case '+':
            val.valS = lhsStr + rhsStr;
            val.valD = 1;
            val.isDecimal = false;
          break;

          default:
            break;
        }
        return &val;
      }
    }
    return &val;
  }

  bool compStr(String str1, String str2){
    if (str1.length() != str2.length()) return false;
    for (int i = 0; i < str1.length(); i++) {
      if (str1[i] == '*' || str2[i] == '*') continue;   //считаем, что если есть подстановочная звезда, то символы равны
      if (str1[i] != str2[i]) return false;
    }
    
    return true;//str1 == str2;
  }

};

/// CallExprAST - Класс узла выражения для вызова команды.
class CallExprAST : public ExprAST {
  String Callee;
  String Cmd;
  std::vector<ExprAST*> Args;
  IoTItem *Item;  // ссылка на объект модуля (прямой доступ к идентификатору указанному в сценарии), если получилось найти модуль по ID
  IoTValue ret;  // хранение возвращаемого значения, т.к. возврат по ссылке осуществляется
  bool ItemIsLocal = false;

public:
  CallExprAST(const String &callee, String &cmd, std::vector<ExprAST*> &args, IoTItem *item)
    : Callee(callee), Cmd(cmd), Args(args), Item(item) {
      if (item) ItemIsLocal = item->iAmLocal;
    }

  IoTValue* exec() {
    if (isIotScenException) return nullptr;   // если прерывание, то сразу выходим
    
    if (Cmd == "exit" || Callee == "exit") {   // если системная команда, то выполняем и выходим
      IoTValue* tmp;
      if (Args.size() > 0 && Args[0]) tmp = Args[0]->exec();
        else SerialPrint("i", "SysExt", "Exit");
      if (tmp) SerialPrint("i", "SysExt", "Exit = '" + tmp->valS + "'");
    
      isIotScenException = true;
      return nullptr;
    }

    if (!ItemIsLocal) Item = findIoTItem(Callee);   // пробуем найти переменную если она не локальная (могла придти по сети в процессе)
    if (!Item) return nullptr; //ret = zeroIotVal;  // если все же не пришла, то либо опечатка, либо уже стерлась - выходим

    // если все же все ок, то готовим параметры для передачи в модуль
    std::vector<IoTValue> ArgsAsIoTValue;
    for (unsigned int i = 0; i < Args.size(); i++) {
      if (Args[i] == nullptr) return nullptr;
      IoTValue *tmp = Args[i]->exec();
      if (tmp != nullptr) ArgsAsIoTValue.push_back(*tmp);
        else return nullptr; //ArgsAsIoTValue.push_back(zeroIotVal);
    }

    ret = Item->execute(Cmd, ArgsAsIoTValue);  // вызываем команду из модуля напрямую с передачей всех аргументов
    
    //if (ret.isDecimal) Serial.printf("Call from  CallExprAST ID = %s, Command = %s, exec result = %f\n", Callee.c_str(), Cmd.c_str(), ret.valD);
    //else Serial.printf("Call from  CallExprAST ID = %s, Command = %s, exec result = %s\n", Callee.c_str(), Cmd.c_str(), ret.valS.c_str());
    return &ret;
  }

  ~CallExprAST() {
    for (unsigned int i = 0; i < Args.size(); i++) {
      if (Args[i]) delete Args[i];
    }
    Args.clear();
    //Serial.printf("Call from  CallExprAST delete\n");
  }
};

/// IfExprAST - Класс узла выражения для if/then/else.
class IfExprAST : public ExprAST {
  ExprAST *Cond, *Then, *Else;
  String _IDNames;

public:
  IfExprAST(ExprAST *cond, ExprAST *then, ExprAST *_else, String *IDNames)
    : Cond(cond), Then(then), Else(_else) {
      if (IDNames) { 
        _IDNames = *IDNames; 
      } else _IDNames = "";
    }

  bool hasEventIdName(String eventIdName) {
      //Serial.printf("Call from  BinaryExprAST _IDNames:%s\n", _IDNames.c_str());
      return _IDNames.indexOf(" " + eventIdName + " ") >= 0;   // определяем встречался ли ИД, для которого исполняем сценарий в выражении IF
  }

  IoTValue* exec() {
    if (isIotScenException) return nullptr;
    IoTValue *res_ret = nullptr;
    IoTValue *cond_ret = nullptr;

    if (Cond) cond_ret = Cond->exec();
    
    if (!cond_ret) {
      //Serial.printf("Call from  IfExprAST: Skip If\n");
      return nullptr; //&zeroIotVal;
    }

    if (cond_ret->isDecimal && cond_ret->valD) {
      if (Then == nullptr) return nullptr;
      res_ret = Then->exec();
    } else { 
      if (Else == nullptr) return nullptr;
      res_ret = Else->exec();
    } 

    //if (!res_ret) Serial.printf("Call from  IfExprAST: Cond result = %f, no body result\n", cond_ret->valD);
    //else if (res_ret->isDecimal) Serial.printf("Call from  IfExprAST: Cond result = %f, result = %f\n", cond_ret->valD, res_ret->valD);
    //else Serial.printf("Call from  IfExprAST: Cond result = %f, result = %s\n", cond_ret->valD, res_ret->valS.c_str());
    Serial.printf("\n");
    return cond_ret;
  }

  ~IfExprAST() {
    if (Cond) delete Cond;
    if (Then) delete Then;
    if (Else) delete Else;
    //Serial.printf("Call from  IfExprAST delete\n");
  }
};

/// BracketsExprAST - Класс узла блока кода {}.
class BracketsExprAST : public ExprAST {
  std::vector<ExprAST*> BracketsList;

public:
  BracketsExprAST(std::vector<ExprAST*> &bracketsList)
    : BracketsList(bracketsList) {}

  IoTValue* exec() {
    if (isIotScenException) return nullptr;
    //Serial.printf("Call from  BracketsExprAST OperCount = %d \n", BracketsList.size());
    
    IoTValue* lastExecValue = nullptr;
    for (unsigned int i = 0; i < BracketsList.size(); i++) {
      if (BracketsList[i] == nullptr) return nullptr; 
      lastExecValue = BracketsList[i]->exec();
    }

    return lastExecValue;
  }

  ~BracketsExprAST() {
    for (unsigned int i = 0; i < BracketsList.size(); i++) {
      if (BracketsList[i]) delete BracketsList[i];
    }
    BracketsList.clear();

    //Serial.printf("Call from  BracketsExprAST delete\n");
  }
};


  //===----------------------------------------------------------------------===//
  // Lexer (Лексический анализатор)
  //===----------------------------------------------------------------------===//


  char IoTScenario::getLastChar() {
    strIterator++;
    return strFromFile->charAt(strIterator - 1);
  }

  /// gettok - Возвращает следующий токен из стандартного потока ввода.
  int IoTScenario::gettok() {

    // Пропускаем пробелы.
    while (isspace(LastChar))
      LastChar = getLastChar();

    if (isalpha(LastChar) || LastChar == '_') { // идентификатор: [a-zA-Z][a-zA-Z0-9]*
      IdentifierStr = LastChar;
      while (isalnum((LastChar = getLastChar())) || LastChar == '_'){
        IdentifierStr += LastChar;
      }

      //Serial.printf("%s ", IdentifierStr.c_str());

      if (IdentifierStr == "if") return tok_if;
      if (IdentifierStr == "then") return tok_then;
      if (IdentifierStr == "else") return tok_else;
      return tok_identifier;
    }

    if (isdigit(LastChar)) {   // Число: [0-9.]+
      String NumStr;
      do {
        NumStr += LastChar;
        LastChar = getLastChar();
      } while (isdigit(LastChar) || LastChar == '.');

      NumVal = strtod(NumStr.c_str(), 0);
      return tok_number;
    }

    if (LastChar == '#') {
      // Комментарий до конца строки
      do LastChar = getLastChar();
      while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
      
      if (LastChar != EOF)
        return gettok();
    }

    if (LastChar == '"') { // "строка"    
      IdentifierStr = "";
      LastChar = getLastChar();
      while (LastChar != '"') {
        IdentifierStr += LastChar;
        LastChar = getLastChar();
      }
      LastChar = getLastChar();
        
      return tok_string;
    }
    
    // Проверка конца файла.
    //if (LastChar == EOF)
    //  return tok_eof;

    if (LastChar == '=') {
      LastChar = getLastChar();  
      if (LastChar == '=') {
        LastChar = getLastChar();
        return tok_equal;
      } else return '=';
    }

    if (LastChar == '!') {
      LastChar = getLastChar();  
      if (LastChar == '=') {
        LastChar = getLastChar();
        return tok_notequal;
      } else return '!';
    }

    if (LastChar == '<') {
      LastChar = getLastChar();  
      if (LastChar == '=') {
        LastChar = getLastChar();
        return tok_lesseq;
      } else return '<';
    }

    if (LastChar == '>') {
      LastChar = getLastChar();  
      if (LastChar == '=') {
        LastChar = getLastChar();
        return tok_greateq;
      } else return '>';
    }

    // В противном случае просто возвращаем символ как значение ASCII
    int ThisChar = LastChar;
    LastChar = getLastChar();
    return ThisChar;
  }


  //===----------------------------------------------------------------------===//
  // Parser (Парсер или Синтаксический Анализатор)
  //===----------------------------------------------------------------------===//


  /// CurTok/getNextToken - Предоставляет простой буфер токенов. CurTok - это текущий
  /// токен, просматриваемый парсером. getNextToken получает следующий токен от
  /// лексического анализатора и обновляет CurTok.
  int IoTScenario::getNextToken() {
    return CurTok = gettok();
  }

  /// GetTokPrecedence - Возвращает приоритет текущего бинарного оператора.
  int IoTScenario::GetTokPrecedence() {
    if (!(isascii(CurTok) || BinopPrecedence.count(CurTok)))
      return -1;
    
    // Удостоверимся, что это объявленный бинарный оператор.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
  }

  /// Error* - Это небольшие вспомогательные функции для обработки ошибок.
  ExprAST* IoTScenario::Error(const char *Str) { Serial.printf("Error: %s\n", Str); return nullptr;}


  /// identifierexpr
  ///   ::= identifier
  ///   ::= identifier '(' expression* ')'
  ExprAST* IoTScenario::ParseIdentifierExpr(String *IDNames) {
    String IdName = IdentifierStr;
    String Cmd = "";
    IoTItem* tmpItem = findIoTItem(IdName);

    getNextToken();  // получаем идентификатор.
    
    if (CurTok == '.') {
      getNextToken();
      Cmd = IdentifierStr;
      getNextToken();
    }

    if (CurTok != '(') { // Обычная переменная.
      // if (tmpItem) return new VariableExprAST(IdName, tmpItem);
      //   else return new StringExprAST("id " + IdName + " not_found");

      // создаем экземпляр переменной в любом случае, даж если не нашли (tmpItem = nulptr), т.к. переменная может придти из сети позже
      return new VariableExprAST(IdName, tmpItem);  
    }
    
    // Вызов функции.
    getNextToken();  // получаем (
    std::vector<ExprAST*> Args;
    if (CurTok != ')') {
      while (1) {
        ExprAST *Arg = ParseExpression(IDNames);
        if (!Arg) return 0;
        Args.push_back(Arg);

        if (CurTok == ')') break;

        if (CurTok != ',')
          return Error("Expected ')' or ',' in argument list");
        getNextToken();
      }
    }

    // Получаем ')'.
    getNextToken();
  
    //if (tmpItem) 
    return new CallExprAST(IdName, Cmd, Args, tmpItem);   // создаем объект запуска функции в любом случае даж если не нашли Item
      //else return new StringExprAST("id " + IdName + " not_found");
  }

  /// numberexpr ::= number
  ExprAST* IoTScenario::ParseNumberExpr() {
    ExprAST *Result = new NumberExprAST(NumVal);
    getNextToken(); // получаем число
    return Result;
  }

  /// parenexpr ::= '(' expression ')'
  ExprAST* IoTScenario::ParseParenExpr() {
    getNextToken();  // получаем (.
    ExprAST *V = ParseExpression(nullptr);
    if (!V) return 0;
    
    if (CurTok != ')')
      return Error("expected ')'");
    getNextToken();  // получаем ).
    return V;
  }

  /// bracketsexpr ::= '{' expression '}'
  ExprAST* IoTScenario::ParseBracketsExpr() {
    getNextToken();  // получаем {.
    std::vector<ExprAST*> bracketsList;
    if (CurTok != '}') {
      while (1) {
        ExprAST *Expr = ParseExpression(nullptr);
        if (!Expr) return 0;
        bracketsList.push_back(Expr);

        if (CurTok != ';')
          return Error("Expected ';' in operation list");
        int ttok = getNextToken();
        if (!ttok) { Error("Expected '}'"); break; }
        
        if (CurTok == '}') break;


      }
    }
 
    getNextToken();  // получаем }.
    return new BracketsExprAST(bracketsList);
  }

  /// quotesexpr ::= '"' expression '"'
  ExprAST* IoTScenario::ParseQuotesExpr() {
    String StringCont = IdentifierStr;
    ExprAST *Result = new StringExprAST(StringCont);
    getNextToken(); // получаем число
    return Result;
  }

  /// ifexpr ::= 'if' expression 'then' expression 'else' expression
  ExprAST* IoTScenario::ParseIfExpr(String *IDNames) {
    getNextToken();  // Получаем if.
    
    // условие.
    ExprAST *Cond = ParseExpression(IDNames);
    if (!Cond) return 0;
    
    if (CurTok != tok_then)
      return Error("expected then");
    getNextToken();  // Получаем then
    
    ExprAST *Then = ParseExpression(nullptr);
    if (!Then) return 0;
    
    //if (CurTok != tok_else)
    //  return Error("expected else");
    ExprAST *Else = nullptr;
    if (CurTok == tok_else) {
      getNextToken();
      Else = ParseExpression(nullptr);
    }

    return new IfExprAST(Cond, Then, Else, IDNames);
  }

  /// primary
  ///   ::= identifierexpr
  ///   ::= numberexpr
  ///   ::= parenexpr
  ExprAST* IoTScenario::ParsePrimary(String* IDNames) {
    switch (CurTok) {
    default: return Error("unknown token when expecting an expression");
    case tok_identifier: {
      if (IDNames) {
        String tmpstr = *IDNames;
        *IDNames = tmpstr + " " + IdentifierStr + " ";
      }
      return ParseIdentifierExpr(IDNames);
    }
    case tok_number:     return ParseNumberExpr();
    case '(':            return ParseParenExpr();
    case '{':            return ParseBracketsExpr();
    case tok_string:     return ParseQuotesExpr();
    case tok_if:         return ParseIfExpr(IDNames);
    }
  }

  /// binoprhs
  ///   ::= ('+' primary)*
  ExprAST* IoTScenario::ParseBinOpRHS(int ExprPrec, ExprAST *LHS, String *IDNames) {
    // Если это бинарный оператор, получаем его приоритет
    while (1) {
      int TokPrec = GetTokPrecedence();
      
      // Если этот бинарный оператор связывает выражения по крайней мере так же, 
      // как текущий, то используем его
      if (TokPrec < ExprPrec)
        return LHS;
      
      // Отлично, мы знаем, что это бинарный оператор.
      int BinOp = CurTok;
      getNextToken();  // eat binop
      
      // Разобрать первичное выражение после бинарного оператора
      ExprAST *RHS = ParsePrimary(IDNames);
      if (!RHS) return 0;
      
      // Если BinOp связан с RHS меньшим приоритетом, чем оператор после RHS, 
      // то берём часть вместе с RHS как LHS.
      int NextPrec = GetTokPrecedence();
      if (TokPrec < NextPrec) {
        RHS = ParseBinOpRHS(TokPrec+1, RHS, IDNames);
        if (RHS == 0) return 0;
      }
      
      // Собираем LHS/RHS.
      LHS = new BinaryExprAST(BinOp, LHS, RHS);
    }
  }
  

  /// expression
  ///   ::= primary binoprhs
  ///
  ExprAST* IoTScenario::ParseExpression(String *IDNames) {
    ExprAST *LHS = ParsePrimary(IDNames);
    if (!LHS) return 0;
    return ParseBinOpRHS(0, LHS, IDNames);
  }

  void IoTScenario::clearScenarioElements() {  // удаляем все корневые элементы дерева AST
    for (unsigned int i = 0; i < ScenarioElements.size(); i++) {
      if (ScenarioElements[i]) delete ScenarioElements[i];
    }
    ScenarioElements.clear();
  }

  void IoTScenario::loadScenario(String fileName) {  // посимвольно считываем и сразу интерпретируем сценарий в дерево AST
    clearScenarioElements();  // удаляем все корневые элементы перед загрузкой новых.
    LastChar = ' ';

    File myfile = seekFile(fileName);
    if (myfile.available()) {
      strFromFile = new String("");
      *strFromFile = myfile.readString();
      myfile.close();

      getNextToken();
      while (strIterator < strFromFile->length()-1) {
        //Serial.printf("-%c", LastChar);
        switch (CurTok) {
          //case tok_eof:    return;
          //case ';':        getNextToken(); break;  // игнорируем верхнеуровневые точки с запятой.
          case tok_if: {
            String IDNames = "";  // накопитель встречающихся идентификаторов в условии
            ScenarioElements.push_back(ParseIfExpr(&IDNames)); 
            //Serial.printf("vvvvvvvvvvvvvvvv %s", IDNames.c_str());
            break;
          }
          default:         getNextToken(); break;
        }
      }
      delete strFromFile;
      strIterator = 0;
    } else {Error("Open file scenario error");}
  }

  void IoTScenario::ExecScenario(String eventIdName) {  // запускаем поочереди все корневые элементы выражений в сценарии, ожидаемо - это IFы
     // eventIdName - ID элемента для которого выполняем сценарий, т.е. игнорируем любые проверки, если нет такого ID в условиях
    isIotScenException = false;
    Serial.printf("Count root elements in scenario: %d\n", ScenarioElements.size());
    for (unsigned int i = 0; i < ScenarioElements.size(); i++) {
      if (ScenarioElements[i] && ScenarioElements[i]->hasEventIdName(eventIdName)) ScenarioElements[i]->exec();
      //else Serial.printf("Call from  ExecScenario: Skip ifexec because %s not found\n", eventIdName.c_str());
      if (isIotScenException) return;
    }
  }


  IoTScenario::IoTScenario() {
    // Задаём стандартные бинарные операторы.
    // 1 - наименьший приоритет.
    BinopPrecedence['='] = 1; 
    BinopPrecedence['|'] = 5; 
    BinopPrecedence['&'] = 6;  
    BinopPrecedence[tok_equal] = 10;  // ==
    BinopPrecedence[tok_notequal] = 11;  // !=
    BinopPrecedence[tok_lesseq] = 15;  // <=
    BinopPrecedence[tok_greateq] = 16;  // >=
    BinopPrecedence['<'] = 20;
    BinopPrecedence['>'] = 21;
    BinopPrecedence['+'] = 25;
    BinopPrecedence['-'] = 26;
    BinopPrecedence['/'] = 27;  
    BinopPrecedence['*'] = 28;  // highest.
  }
  
  IoTScenario::~IoTScenario() {}