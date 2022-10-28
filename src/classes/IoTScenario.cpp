#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTScenario.h"
#include "utils/FileUtils.h"
#include "NTP.h"

bool isIotScenException = false;  // признак исключения и попытки прекратить выполнение сценария заранее

// Лексический анализатор возвращает токены [0-255], если это неизвестны,
// иначе одну из известных единиц кода
enum Token {
    tok_eof = -1,

    // операнды (первичные выражения: идентификаторы, числа)
    tok_identifier = -4,
    tok_number = -5,
    tok_string = -3,

    // двухсимвольные операторы бинарных операций
    tok_equal = -2,
    tok_notequal = -9,
    tok_lesseq = -10,
    tok_greateq = -11,
    tok_silentset = -12,

    // управление
    tok_if = -6,
    tok_then = -7,
    tok_else = -8
};

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (Абстрактное Синтаксическое Дерево или Дерево Парсинга)
//===----------------------------------------------------------------------===//

/// ExprAST - Базовый класс для всех узлов выражений.
ExprAST::~ExprAST() {}
IoTValue *ExprAST::exec() { return nullptr; }
int ExprAST::setValue(IoTValue *val, bool generateEvent) { return 0; }                  // 0 - установка значения не поддерживается наследником
bool ExprAST::hasEventIdName(String eventIdName) { return false; }  // по умолчанию все узлы не связаны с ИД события, для которого выполняется сценарий
// struct IoTValue zeroIotVal;

/// NumberExprAST - Класс узла выражения для числовых литералов (Например, "1.0").
class NumberExprAST : public ExprAST {
    IoTValue Val;

   public:
    NumberExprAST(String val) {
        Val.valD = strtod(val.c_str(), 0);
        Val.valS = val;
    }

    IoTValue *exec() {
        if (isIotScenException) return nullptr;
        // Serial.printf("Call from  NumberExprAST: %f\n", Val.valD);
        return &Val;
    }
};

/// StringExprAST - Класс узла выражения для строковых литералов (Например, "Example with spaces and quotes").
class StringExprAST : public ExprAST {
    IoTValue Val;

   public:
    StringExprAST(String val) {
        Val.isDecimal = false;
        Val.valS = val;
    }

    IoTValue *exec() {
        if (isIotScenException) return nullptr;
        // Serial.printf("Call from  StringExprAST: %s\n", Val.valS.c_str());
        return &Val;
    }
};

/// VariableExprAST - Класс узла выражения для переменных (например, "a").
class VariableExprAST : public ExprAST {
    String Name;
    IoTItem *Item;  // ссылка на объект модуля (прямой доступ к идентификатору указанному в сценарии), если получилось найти модуль по ID
    bool ItemIsLocal = false;

   public:
    VariableExprAST(const String &name, IoTItem *item) : Name(name), Item(item) {
        if (item) ItemIsLocal = item->iAmLocal;
    }

    int setValue(IoTValue *val, bool generateEvent) {
        if (!ItemIsLocal) Item = findIoTItem(Name);
        if (Item)
            Item->setValue(*val, generateEvent);
        else
            return 0;

        return 1;
    }

    IoTValue *exec() {
        if (isIotScenException) return nullptr;
        if (!ItemIsLocal) Item = findIoTItem(Name);
        if (Item) {
            // if (Item->value.isDecimal)
            //   Serial.printf("Call from  VariableExprAST: %s = %f\n", Name.c_str(), Item->value.valD);
            // else Serial.printf("Call from  VariableExprAST: %s = %s\n", Name.c_str(), Item->value.valS.c_str());
            return &(Item->value);
        }

        SerialPrint("E", Name, "Элемент не найден или соединение потеряно", Name);
        return nullptr;  // Item не найден.
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
        // Serial.printf("Call from  BinaryExprAST delete\n");
    }

    IoTValue *exec() {
        if (isIotScenException) return nullptr;
        // String printStr = "";

        // if (Op == tok_equal)
        //     printStr = "==";
        // else if (Op == tok_notequal)
        //     printStr = "!=";
        // else if (Op == tok_lesseq)
        //     printStr = "<=";
        // else if (Op == tok_greateq)
        //     printStr = ">=";
        // else
        //     printStr = printStr + (char)Op;

        // Serial.printf("Call from  BinaryExprAST: %s\n", printStr.c_str());

        if (RHS == nullptr || LHS == nullptr) return nullptr;

        IoTValue *rhs = RHS->exec();  // получаем значение правого операнда для возможного использования в операции присваивания
        if (rhs == nullptr) return nullptr;

        if (Op == '=' && LHS->setValue(rhs, true)) {  // если установка значения не поддерживается, т.е. слева не переменная, то работаем по другим комбинациям далее
            return rhs;                         // иначе возвращаем присвоенное значение справа
        }

        if (Op == tok_silentset && LHS->setValue(rhs, false)) {  // если установка значения не поддерживается, т.е. слева не переменная, то работаем по другим комбинациям далее
            return rhs;                         // иначе возвращаем присвоенное значение справа
        }

        IoTValue *lhs = LHS->exec();  // если присваивания не произошло, значит операция иная и необходимо значение левого операнда
        if (lhs == nullptr) return nullptr;

        
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
                    if (rhs->valD != 0)
                        val.valD = lhs->valD / rhs->valD;
                    else
                        val.valD = 3.4E+38;
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
            if (lhs->isDecimal)
                lhsStr = (String)lhs->valD;
            else
                lhsStr = lhs->valS;
            
            if (rhs->isDecimal)
                rhsStr = (String)rhs->valD;
            else
                rhsStr = rhs->valS;
            
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
        
        return &val;
    }

    bool compStr(String str1, String str2) {
        if (str1.length() != str2.length()) return false;
        for (int i = 0; i < str1.length(); i++) {
            if (str1[i] == '*' || str2[i] == '*') continue;  //считаем, что если есть подстановочная звезда, то символы равны
            if (str1[i] != str2[i]) return false;
        }

        return true;  // str1 == str2;
    }
};

/// CallExprAST - Класс узла выражения для вызова команды.
class CallExprAST : public ExprAST {
    String Callee;
    String Cmd;
    std::vector<ExprAST *> Args;
    IoTItem *Item;  // ссылка на объект модуля (прямой доступ к идентификатору указанному в сценарии), если получилось найти модуль по ID
    IoTValue ret;   // хранение возвращаемого значения, т.к. возврат по ссылке осуществляется
    bool ItemIsLocal = false;

   public:
    CallExprAST(const String &callee, String &cmd, std::vector<ExprAST *> &args, IoTItem *item)
        : Callee(callee), Cmd(cmd), Args(args), Item(item) {
        if (item) ItemIsLocal = item->iAmLocal;
    }

    IoTValue *exec() {
        if (isIotScenException) return nullptr;  // если прерывание, то сразу выходим

        if (Cmd == "exit" || Callee == "exit") {  // если системная команда, то выполняем и выходим
            IoTValue *tmp;
            if (Args.size() > 0 && Args[0])
                tmp = Args[0]->exec();
            else
                SerialPrint("i", "SysExt", "Exit");
            if (tmp) SerialPrint("i", "SysExt", "Exit = '" + tmp->valS + "'");

            isIotScenException = true;
            return nullptr;
        }

        if (!ItemIsLocal) Item = findIoTItem(Callee);  // пробуем найти переменную если она не локальная (могла придти по сети в процессе)
        if (!Item) return nullptr;                     // ret = zeroIotVal;  // если все же не пришла, то либо опечатка, либо уже стерлась - выходим

        // если все же все ок, то готовим параметры для передачи в модуль
        std::vector<IoTValue> ArgsAsIoTValue;
        for (unsigned int i = 0; i < Args.size(); i++) {
            if (Args[i] == nullptr) return nullptr;
            IoTValue *tmp = Args[i]->exec();
            if (tmp != nullptr)
                ArgsAsIoTValue.push_back(*tmp);
            else
                return nullptr;  // ArgsAsIoTValue.push_back(zeroIotVal);
        }

        ret = Item->execute(Cmd, ArgsAsIoTValue);  // вызываем команду из модуля напрямую с передачей всех аргументов

        // if (ret.isDecimal) Serial.printf("Call from  CallExprAST ID = %s, Command = %s, exec result = %f\n", Callee.c_str(), Cmd.c_str(), ret.valD);
        // else Serial.printf("Call from  CallExprAST ID = %s, Command = %s, exec result = %s\n", Callee.c_str(), Cmd.c_str(), ret.valS.c_str());
        return &ret;
    }

    ~CallExprAST() {
        for (unsigned int i = 0; i < Args.size(); i++) {
            if (Args[i]) delete Args[i];
        }
        Args.clear();
        // Serial.printf("Call from  CallExprAST delete\n");
    }
};

// Для сокращения количества преобразований используем числовые коды для фиксации названия системной функции,
// которые поддерживает прошивка
enum SysOp {
    sysop_notfound = 0,
    sysop_reboot = 1,
    sysop_digitalRead,
    sysop_analogRead,     //
    sysop_digitalWrite,   //
    sysop_digitalInvert,  //
    sysop_deepSleep,      //
    sysop_getHours,       //
    sysop_getMinutes,     //
    sysop_getSeconds,     //
    sysop_getMonth,       //
    sysop_getDay,
    sysop_gethhmm,
    sysop_gethhmmss,
    sysop_getTime,
    sysop_getRSSI,
    sysop_getIP,
    sysop_mqttPub,
    sysop_getUptime
};

IoTValue sysExecute(SysOp command, std::vector<IoTValue> &param) {
    IoTValue value = {};

    if (_time_isTrust)
        switch (command) {
            case sysop_getHours:
                value.valD = _time_local.hour;
                break;
            case sysop_getMinutes:
                value.valD = _time_local.minute;
                break;
            case sysop_getSeconds:
                value.valD = _time_local.second;
                break;
            case sysop_getMonth:
                value.valD = _time_local.month;
                break;
            case sysop_getDay:
                value.valD = _time_local.day_of_month;
                break;
            case sysop_gethhmm:
                value.isDecimal = false;
                value.valS = getTimeLocal_hhmm();
                break;
            case sysop_gethhmmss:
                value.isDecimal = false;
                value.valS = getTimeLocal_hhmmss();
                break;
            case sysop_getTime:
                value.isDecimal = false;
                value.valS = getDateTimeDotFormated();
                break;
        }
    else {
        value.isDecimal = false;
        value.valS = "none";
    }

    switch (command) {
        case sysop_reboot:
            ESP.restart();
            break;
        case sysop_digitalRead:
            if (param.size()) {
                IoTgpio.pinMode(param[0].valD, INPUT);
                value.valD = IoTgpio.digitalRead(param[0].valD);
            }
            break;
        case sysop_analogRead:
            if (param.size()) {
                IoTgpio.pinMode(param[0].valD, INPUT);
                value.valD = IoTgpio.analogRead(param[0].valD);
            }
            break;
        case sysop_digitalWrite:
            if (param.size() == 2) {
                IoTgpio.pinMode(param[0].valD, OUTPUT);
                IoTgpio.digitalWrite(param[0].valD, param[1].valD);
            }
            break;
        case sysop_digitalInvert:
            if (param.size()) {
                IoTgpio.pinMode(param[0].valD, OUTPUT);
                IoTgpio.digitalInvert(param[0].valD);
            }
            break;
        case sysop_deepSleep:
            if (param.size()) {
                Serial.printf("Ушел спать на %d сек...", (int)param[0].valD);
#ifdef ESP32
                esp_sleep_enable_timer_wakeup(param[0].valD * 1000000);
                delay(1000);
                esp_deep_sleep_start();
#else
                ESP.deepSleep(param[0].valD * 1000000);
#endif
            }
            break;
        case sysop_getRSSI:
            value.valD = WiFi.RSSI();
            value.isDecimal = true;
            break;
        case sysop_getIP:
            value.valS = jsonReadStr(settingsFlashJson, F("ip"));
            value.isDecimal = false;
            break;
        case sysop_mqttPub:
            if (param.size() == 2) {
                // Serial.printf("Call from  sysExecute %s %s\n", param[0].valS.c_str(), param[1].valS.c_str());
                String tmpStr = param[1].valS;
                if (param[1].isDecimal) tmpStr = param[1].valD;
                value.valD = mqtt.publish(param[0].valS.c_str(),  tmpStr.c_str(), false);
            }
            break;
        case sysop_getUptime:
            value.valS = jsonReadStr(errorsHeapJson, F("upt"));
            value.isDecimal = false;
            break;
    }

    return value;
}

/// SysCallExprAST - Класс узла выражения для вызова системных команд.
class SysCallExprAST : public ExprAST {
    String Callee;
    std::vector<ExprAST *> Args;
    IoTValue ret;  // хранение возвращаемого значения, т.к. возврат по ссылке осуществляется
    // bool ItemIsLocal = false;
    SysOp operation;

   public:
    SysCallExprAST(const String &callee, std::vector<ExprAST *> &args)
        : Callee(callee), Args(args) {
        if (Callee == "reboot")
            operation = sysop_reboot;
        else if (Callee == "digitalRead")
            operation = sysop_digitalRead;
        else if (Callee == "analogRead")
            operation = sysop_analogRead;
        else if (Callee == "digitalWrite")
            operation = sysop_digitalWrite;
        else if (Callee == "digitalInvert")
            operation = sysop_digitalInvert;
        else if (Callee == "deepSleep")
            operation = sysop_deepSleep;
        else if (Callee == "getTime")
            operation = sysop_getTime;
        else if (Callee == "getHours")
            operation = sysop_getHours;
        else if (Callee == "getMinutes")
            operation = sysop_getMinutes;
        else if (Callee == "getSeconds")
            operation = sysop_getSeconds;
        else if (Callee == "getMonth")
            operation = sysop_getMonth;
        else if (Callee == "getDay")
            operation = sysop_getDay;
        else if (Callee == "getRSSI")
            operation = sysop_getRSSI;
        else if (Callee == "getIP")
            operation = sysop_getIP;
        else if (Callee == "mqttPub")
            operation = sysop_mqttPub;
        else if (Callee == "gethhmm")
            operation = sysop_gethhmm;
        else if (Callee == "gethhmmss")
            operation = sysop_gethhmmss;
        else if (Callee == "getTime")
            operation = sysop_getTime;
        else if (Callee == "getUptime")
            operation = sysop_getUptime;
        else
            operation = sysop_notfound;
    }

    IoTValue *exec() {
        // Serial.printf("Call from  SysCallExprAST exec %d\n", operation);

        if (isIotScenException) return nullptr;  // если прерывание, то сразу выходим

        // готовим параметры для передачи в в функцию интерпретатор действий
        std::vector<IoTValue> ArgsAsIoTValue;
        for (unsigned int i = 0; i < Args.size(); i++) {
            if (Args[i] == nullptr) return nullptr;
            IoTValue *tmp = Args[i]->exec();
            if (tmp != nullptr)
                ArgsAsIoTValue.push_back(*tmp);
            else
                return nullptr;
        }

        ret = sysExecute(operation, ArgsAsIoTValue);  // вызываем функцию интерпретатор с передачей всех аргументов

        return &ret;
    }

    ~SysCallExprAST() {
        for (unsigned int i = 0; i < Args.size(); i++) {
            if (Args[i]) delete Args[i];
        }
        Args.clear();
        // Serial.printf("Call from  CallExprAST delete\n");
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
        } else
            _IDNames = "";
    }

    bool hasEventIdName(String eventIdName) {
        // Serial.printf("Call from  BinaryExprAST _IDNames:%s\n", _IDNames.c_str());
        return _IDNames.indexOf(" " + eventIdName + " ") >= 0;  // определяем встречался ли ИД, для которого исполняем сценарий в выражении IF
    }

    IoTValue *exec() {
        if (isIotScenException) return nullptr;
        IoTValue *res_ret = nullptr;
        IoTValue *cond_ret = nullptr;

        if (Cond) cond_ret = Cond->exec();

        if (!cond_ret) {
            // Serial.printf("Call from  IfExprAST: Skip If\n");
            return nullptr;  //&zeroIotVal;
        }

        // если число больше нуля или строка не равна пустой, то считаем условие выполненным
        if (cond_ret->isDecimal && cond_ret->valD || !(cond_ret->isDecimal) && cond_ret->valS != "") {
            if (Then == nullptr) return nullptr;
            res_ret = Then->exec();
        } else {
            if (Else == nullptr) return nullptr;
            res_ret = Else->exec();
        }

        // if (!res_ret) Serial.printf("Call from  IfExprAST: Cond result = %f, no body result\n", cond_ret->valD);
        // else if (res_ret->isDecimal) Serial.printf("Call from  IfExprAST: Cond result = %f, result = %f\n", cond_ret->valD, res_ret->valD);
        // else Serial.printf("Call from  IfExprAST: Cond result = %f, result = %s\n", cond_ret->valD, res_ret->valS.c_str());
        //Serial.printf("\n");
        return cond_ret;
    }

    ~IfExprAST() {
        if (Cond) delete Cond;
        if (Then) delete Then;
        if (Else) delete Else;
        // Serial.printf("Call from  IfExprAST delete\n");
    }
};

/// BracketsExprAST - Класс узла блока кода {}.
class BracketsExprAST : public ExprAST {
    std::vector<ExprAST *> BracketsList;

   public:
    BracketsExprAST(std::vector<ExprAST *> &bracketsList)
        : BracketsList(bracketsList) {}

    IoTValue *exec() {
        if (isIotScenException) return nullptr;
        // Serial.printf("Call from  BracketsExprAST OperCount = %d \n", BracketsList.size());

        IoTValue *lastExecValue = nullptr;
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

        // Serial.printf("Call from  BracketsExprAST delete\n");
    }
};

//===----------------------------------------------------------------------===//
// Lexer (Лексический анализатор)
//===----------------------------------------------------------------------===//

int IoTScenario::getLastChar() {
    if (mode == 0) {
        if (file) {
            LastChar = file.read();
            if (LastChar == 10) curLine++;
            return LastChar;
        } else
            return EOF;
    } else if (mode == 1) {
        if (charCount < strFromFile.length()) {
            LastChar = strFromFile.charAt(charCount);
            // Serial.printf("%d, ", LastChar);
            if (LastChar == 10) curLine++;
            charCount++;
            return LastChar;
        } else
            return EOF;
    } else {
        return EOF;
    }
}

/// gettok - Возвращает следующий токен из стандартного потока ввода.
int IoTScenario::gettok() {
    // Пропускаем пробелы.
    while (isspace(LastChar) || LastChar == ';')
        LastChar = getLastChar();

    if (isalpha(LastChar) || LastChar == '_') {  // идентификатор: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = (char)LastChar;
        while (isalnum((LastChar = getLastChar())) || LastChar == '_') {
            IdentifierStr += (char)LastChar;
        }

        // Serial.printf("%s ", IdentifierStr.c_str());

        if (IdentifierStr == "if") return tok_if;
        if (IdentifierStr == "then") return tok_then;
        if (IdentifierStr == "else") return tok_else;
        return tok_identifier;
    }

    NumStr = "";
    if (LastChar == '-') {
        LastChar = getLastChar();
        if (isdigit(LastChar))
            NumStr = "-";
        else
            return '-';
    }
    if (isdigit(LastChar)) {  // Число: [0-9.]+
        do {
            NumStr += (char)LastChar;
            LastChar = getLastChar();
        } while (isdigit(LastChar) || LastChar == '.');

        return tok_number;
    }

    if (LastChar == '#') {
        // Комментарий до конца строки
        do LastChar = getLastChar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }

    if (LastChar == '"') {  // "строка"
        IdentifierStr = "";
        LastChar = getLastChar();
        while (LastChar != '"' && LastChar != EOF) {
            IdentifierStr += (char)LastChar;
            LastChar = getLastChar();
        }
        LastChar = getLastChar();

        return tok_string;
    }

    // Проверка конца файла.
    if (LastChar == EOF)
        return tok_eof;

    if (LastChar == '=') {
        LastChar = getLastChar();
        if (LastChar == '=') {
            LastChar = getLastChar();
            return tok_equal;
        } else
            return '=';
    }

    if (LastChar == ':') {
        LastChar = getLastChar();
        if (LastChar == '=') {
            LastChar = getLastChar();
            return tok_silentset;
        } else
            return ':';
    }

    if (LastChar == '!') {
        LastChar = getLastChar();
        if (LastChar == '=') {
            LastChar = getLastChar();
            return tok_notequal;
        } else
            return '!';
    }

    if (LastChar == '<') {
        LastChar = getLastChar();
        if (LastChar == '=') {
            LastChar = getLastChar();
            return tok_lesseq;
        } else
            return '<';
    }

    if (LastChar == '>') {
        LastChar = getLastChar();
        if (LastChar == '=') {
            LastChar = getLastChar();
            return tok_greateq;
        } else
            return '>';
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
ExprAST *IoTScenario::Error(const char *Str) {
    Serial.printf("Scenario error in line %d: %s\n", curLine, Str);
    isIotScenException = true;
    return nullptr;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
ExprAST *IoTScenario::ParseIdentifierExpr(String *IDNames, bool callFromCondition) {
    if (isIotScenException) return nullptr;

    String IdName = IdentifierStr;
    String Cmd = "";
    IoTItem *tmpItem = findIoTItem(IdName);

    getNextToken();  // получаем идентификатор.

    if (CurTok == '.') {
        getNextToken();
        Cmd = IdentifierStr;
        getNextToken();
    }

    if (CurTok != '(') {  // Обычная переменная.
        // if (tmpItem) return new VariableExprAST(IdName, tmpItem);
        //   else return new StringExprAST("id " + IdName + " not_found");

        // создаем экземпляр переменной в любом случае, даж если не нашли (tmpItem = nulptr), т.к. переменная может придти из сети позже
        return new VariableExprAST(IdName, tmpItem);
    }

    // Вызов функции.
    getNextToken();  // получаем (
    std::vector<ExprAST *> Args;
    if (CurTok != ')') {
        while (1) {
            ExprAST *Arg = ParseExpression(IDNames, callFromCondition);
            if (!Arg) return nullptr;
            Args.push_back(Arg);

            if (CurTok == ')') break;

            if (CurTok != ',') {
                return Error("Expected ')' or ',' in argument list");
            }
            getNextToken();
        }
    }

    // Получаем ';'.
    getNextToken();

    if (Cmd == "")
        return new SysCallExprAST(IdName, Args);  // создаем объект запуска системной функции
    else
        return new CallExprAST(IdName, Cmd, Args, tmpItem);  // создаем объект запуска функции в любом случае даж если не нашли Item
}

/// numberexpr ::= number
ExprAST *IoTScenario::ParseNumberExpr() {
    if (isIotScenException) return nullptr;

    ExprAST *Result = new NumberExprAST(NumStr);
    getNextToken();  // получаем число
    return Result;
}

/// parenexpr ::= '(' expression ')'
ExprAST *IoTScenario::ParseParenExpr(String *IDNames, bool callFromCondition) {
    getNextToken();  // получаем (.
    ExprAST *V = ParseExpression(IDNames, callFromCondition);
    if (!V) return nullptr;

    if (CurTok != ')')
        return Error("expected ')'");
    // getNextToken();  // получаем ).
    return V;
}

/// bracketsexpr ::= '{' expression '}'
ExprAST *IoTScenario::ParseBracketsExpr(String *IDNames, bool callFromCondition) {
    if (isIotScenException) return nullptr;

    getNextToken();  // получаем {.
    std::vector<ExprAST *> bracketsList;

    while (CurTok != '}') {
        ExprAST *Expr = ParseExpression(IDNames, callFromCondition);
        if (!Expr) return nullptr;
        bracketsList.push_back(Expr);

        // if (CurTok == '}') break;

        // Serial.printf("ParseBracketsExpr CurTok = %d \n", CurTok);

        // if (CurTok != ';')
        //     return Error("Expected ';' in operation list");

        // int ttok = getNextToken();
        if (CurTok == tok_eof) {
            return Error("Expected '}'");
        }
    }

    getNextToken();  // получаем }.
    return new BracketsExprAST(bracketsList);
}

/// quotesexpr ::= '"' expression '"'
ExprAST *IoTScenario::ParseQuotesExpr() {
    if (isIotScenException) return nullptr;

    String StringCont = IdentifierStr;
    ExprAST *Result = new StringExprAST(StringCont);
    getNextToken();  // получаем число
    return Result;
}

/// ifexpr ::= 'if' expression 'then' expression 'else' expression
ExprAST *IoTScenario::ParseIfExpr(String *IDNames) {
    if (isIotScenException) return nullptr;

    getNextToken();  // Получаем if.

    // условие.
    ExprAST *Cond = ParseExpression(IDNames, true);
    if (!Cond) return nullptr;

    if (CurTok != tok_then)
        return Error("expected then");
    getNextToken();  // Получаем then

    ExprAST *Then = ParseExpression(IDNames, false);
    if (!Then) return nullptr;

    // if (CurTok != tok_else)
    //   return Error("expected else");
    ExprAST *Else = nullptr;
    if (CurTok == tok_else) {
        getNextToken();
        Else = ParseExpression(IDNames, false);
        if (!Else) return nullptr;
    }

    return new IfExprAST(Cond, Then, Else, IDNames);
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
ExprAST *IoTScenario::ParsePrimary(String *IDNames, bool callFromCondition) {
    switch (CurTok) {
        default:
            Serial.println(CurTok);
            return Error("unknown token when expecting an expression");
        case tok_identifier: {
            if (callFromCondition && IDNames) {
                String tmpstr = *IDNames;
                *IDNames = tmpstr + " " + IdentifierStr + " ";
            }
            return ParseIdentifierExpr(IDNames, callFromCondition);
        }
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr(IDNames, callFromCondition);
        case '{':
            return ParseBracketsExpr(IDNames, callFromCondition);
        case tok_string:
            return ParseQuotesExpr();
        case tok_if:
            return ParseIfExpr(IDNames);
    }
}

/// binoprhs
///   ::= ('+' primary)*
ExprAST *IoTScenario::ParseBinOpRHS(int ExprPrec, ExprAST *LHS, String *IDNames, bool callFromCondition) {
    if (isIotScenException) return nullptr;

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
        ExprAST *RHS = ParsePrimary(IDNames, callFromCondition);
        if (!RHS) return nullptr;

        // Если BinOp связан с RHS меньшим приоритетом, чем оператор после RHS,
        // то берём часть вместе с RHS как LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, RHS, IDNames, callFromCondition);
            if (RHS == nullptr) return nullptr;
        }

        // Собираем LHS/RHS.
        LHS = new BinaryExprAST(BinOp, LHS, RHS);
    }
}

/// expression
///   ::= primary binoprhs
///
ExprAST *IoTScenario::ParseExpression(String *IDNames, bool callFromCondition) {
    ExprAST *LHS = ParsePrimary(IDNames, callFromCondition);
    if (!LHS) return nullptr;
    return ParseBinOpRHS(0, LHS, IDNames, callFromCondition);
}

void IoTScenario::loadScenario(String fileName) {  // подготавливаем контекст для чтения и интерпретации файла
    isIotScenException = false;

    if (mode == 0) {
        if (file) file.close();
        file = FileFS.open(fileName.c_str(), "r");
        if (!file) {
            Error("Open file scenario error");
            return;
        }
    } else if (mode == 1) {
        file = FileFS.open(fileName.c_str(), "r");
        if (!file) {
            Error("Open file scenario error");
            return;
        }
        strFromFile = file.readString();
        // strFromFile.replace("{\"scen\":\"", "");
        // strFromFile.replace("\\n", "\n");
        // strFromFile.replace("\\\"", "\"");
        // strFromFile.replace(";", " ");
        // strFromFile.replace("\\t", " ");
        // strFromFile.remove(strFromFile.length() - 2, 2);
        Serial.printf("strFromFile: %s, %s\n", strFromFile.c_str(), fileName.c_str());
        file.close();
    }
}

void IoTScenario::exec(String eventIdName) {  // посимвольно считываем и сразу интерпретируем сценарий в дерево AST
    if (mode == 0 && !file) return;

    LastChar = 0;
    CurTok = 0;
    curLine = 1;
    charCount = 0;

    if (mode == 0) file.seek(0);

    if (mode < 2) {
        while (CurTok != EOF) {
            switch (CurTok) {
                case tok_if: {
                    IDNames = "";  // сбрасываем накопитель встречающихся идентификаторов в условии
                    ExprAST *tmpAST = ParseIfExpr(&IDNames);
                    if (tmpAST) {
                        if (tmpAST->hasEventIdName(eventIdName)) {
                            tmpAST->exec();
                        }
                        delete tmpAST;
                    } else
                        getNextToken();
                    break;
                }
                default:
                    getNextToken();
                    break;
            }
        }
    }
}

IoTScenario::IoTScenario() {
    // Задаём стандартные бинарные операторы.
    // 1 - наименьший приоритет.
    BinopPrecedence[tok_silentset] = 1;
    BinopPrecedence['='] = 2;
    BinopPrecedence['|'] = 5;
    BinopPrecedence['&'] = 6;
    BinopPrecedence[tok_equal] = 10;     // ==
    BinopPrecedence[tok_notequal] = 11;  // !=
    BinopPrecedence[tok_lesseq] = 15;    // <=
    BinopPrecedence[tok_greateq] = 16;   // >=
    BinopPrecedence['<'] = 20;
    BinopPrecedence['>'] = 21;
    BinopPrecedence['+'] = 25;
    BinopPrecedence['-'] = 26;
    BinopPrecedence['/'] = 27;
    BinopPrecedence['*'] = 28;  // highest.
}

IoTScenario::~IoTScenario() {
    if (file) file.close();
}
