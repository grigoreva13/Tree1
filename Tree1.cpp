#include <iostream>
#include <unordered_map>

// Базовый класс для всех узлов AST
struct Expression
{
    virtual Expression* transform(Transformer* tr) const = 0;
    virtual ~Expression() {}
};

// Число
struct Number : Expression
{
    Number(double value) : value_(value) {}
    double value() const { return value_; }
    virtual Expression* transform(Transformer* tr) const override { return tr->transformNumber(this); }
private:
    double value_;
};

// Бинарная операция
struct BinaryOperation : Expression
{
    enum OpType { PLUS, MINUS, MUL, DIV };
    BinaryOperation(Expression* left, OpType op, Expression* right) : left_(left), op_(op), right_(right) {}
    Expression* left() const { return left_; }
    OpType operation() const { return op_; }
    Expression* right() const { return right_; }
    virtual Expression* transform(Transformer* tr) const override { return tr->transformBinaryOperation(this); }
private:
    Expression* left_;
    OpType op_;
    Expression* right_;
};

// Вызов функции
struct FunctionCall : Expression
{
    FunctionCall(std::string name, Expression* arg) : name_(name), arg_(arg) {}
    std::string name() const { return name_; }
    Expression* arg() const { return arg_; }
    virtual Expression* transform(Transformer* tr) const override { return tr->transformFunctionCall(this); }
private:
    std::string name_;
    Expression* arg_;
};

// Переменная
struct Variable : Expression
{
    Variable(std::string name) : name_(name) {}
    std::string name() const { return name_; }
    virtual Expression* transform(Transformer* tr) const override { return tr->transformVariable(this); }
private:
    std::string name_;
};

// Класс-трансформер для копирования узлов AST
class Transformer
{
public:
    virtual ~Transformer() {}
    virtual Expression* transformNumber(Number const* number) = 0;
    virtual Expression* transformBinaryOperation(BinaryOperation const* binop) = 0;
    virtual Expression* transformFunctionCall(FunctionCall const* fcall) = 0;
    virtual Expression* transformVariable(Variable const* var) = 0;
};

// Класс, реализующий копирование узлов AST
struct CopySyntaxTree : Transformer
{
    Expression* transformNumber(Number const* number) override
    {
        return new Number(number->value());
    }

    Expression* transformBinaryOperation(BinaryOperation const* binop) override
    {
        Expression* left = binop->left()->transform(this);
        Expression* right = binop->right()->transform(this);
        return new BinaryOperation(left, binop->operation(), right);
    }

    Expression* transformFunctionCall(FunctionCall const* fcall) override
    {
        Expression* arg = fcall->arg()->transform(this);
        return new FunctionCall(fcall->name(), arg);
    }

    Expression* transformVariable(Variable const* var) override
    {
        return new Variable(var->name());
    }
};

int main()
{
    Number* n32 = new Number(32.0);
    Number* n16 = new Number(16.0);
    BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
    FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
    Variable* var = new Variable("var");
    BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
    FunctionCall* callAbs = new FunctionCall("abs", mult);
    CopySyntaxTree CST;
    Expression* newExpr = callAbs->transform(&CST);


    return 0;
}