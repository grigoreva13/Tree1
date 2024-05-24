#include <iostream>
#include <cassert>
#include <string> 
using namespace std;

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;

struct Expression
{
    virtual ~Expression() { }
    virtual double evaluate() const = 0;
    virtual Expression* transform(Transformer* tr) const = 0;
    virtual std::string print() const = 0;
};

struct Transformer
{
    virtual ~Transformer() {}
    virtual Expression* transformNumber(Number const*) = 0;
    virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
    virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
    virtual Expression* transformVariable(Variable const*) = 0;
};

struct Number : Expression
{
    Number(double value) : value_(value) {}
    double value() const { return value_; }
    double evaluate() const { return value_; }
    ~Number() {}
    std::string print() const { return std::to_string(this->value_); }
    Expression* transform(Transformer* tr) const
    {
        return tr->transformNumber(this);
    }
private:
    double value_;
};


struct BinaryOperation : Expression
{
    enum {
        PLUS = '+',
        MINUS = '-',
        DIV = '/',
        MUL = '*'
    };

    BinaryOperation(Expression const* left, int op, Expression const* right) : left_(left), op_(op), right_(right)
    {
        assert(left_ && right_);
    }

    Expression const* left() const { return left_; }
    Expression const* right() const { return right_; }
    int operation() const { return op_; }

    double evaluate() const
    {
        double left = left_->evaluate();
        double right = right_->evaluate();
        switch (op_)
        {
        case PLUS: return left + right;
        case MINUS: return left - right;
        case DIV: return left / right;
        case MUL: return left * right;
        }
    }

    Expression* transform(Transformer* tr) const
    {
        return tr->transformBinaryOperation(this);
    }

    std::string print() const {
        return this->left_->print() + std::string(1, this->op_) + this->right_->print();
    }
private:
    Expression const* left_;
    Expression const* right_;
    int op_;
};

struct FunctionCall : Expression
{
    FunctionCall(std::string const& name, Expression const* arg) : name_(name), arg_(arg)
    {
        assert(arg_);
        assert(name_ == "sqrt" || name_ == "abs");
    }

    std::string const& name() const
    {
        return name_;
    }

    Expression const* arg() const
    {
        return arg_;
    }

    ~FunctionCall() { delete arg_; }

    virtual double evaluate() const {
        if (name_ == "sqrt")
            return sqrt(arg_->evaluate());
        else return fabs(arg_->evaluate());
    }

    std::string print() const {
        return this->name_ + "(" + this->arg_->print() + ")";
    }

    Expression* transform(Transformer* tr) const
    {
        return tr->transformFunctionCall(this);
    }
private:
    std::string const name_;
    Expression const* arg_;
};

struct Variable : Expression
{
    Variable(std::string const& name) : name_(name) { }

    std::string const& name() const { return name_; }

    double evaluate() const
    {
        return 0.0;
    }

    std::string print() const {
        return this->name_;
    }

    Expression* transform(Transformer* tr) const
    {
        return tr->transformVariable(this);
    }
private:
    std::string const name_;
};

struct CopySyntaxTree : Transformer
{
    Expression* transformNumber(Number const* number)
    {
        Expression* exp = new Number(number->value());
        return exp;
    }

    Expression* transformBinaryOperation(BinaryOperation const* binop)
    {
        Expression* exp = new BinaryOperation((binop->left())->transform(this), binop->operation(), (binop->right())->transform(this));
        return exp;
    }

    Expression* transformFunctionCall(FunctionCall const* fcall)
    {
        Expression* exp = new FunctionCall(fcall->name(), (fcall->arg())->transform(this));
        return exp;
    }

    Expression* transformVariable(Variable const* var)
    {
        Expression* exp = new Variable(var->name());
        return exp;
    }

    ~CopySyntaxTree() { };
};

struct FoldConstants : Transformer
{
    Expression* transformNumber(Number const* number)
    {
        Expression* exp = new Number(number->value());
        return exp;
    }

    Expression* transformBinaryOperation(BinaryOperation const* binop)
    {
        Expression* nleft = (binop->left())->transform(this);
        Expression* nright = (binop->right())->transform(this);
        int noperation = binop->operation();

        BinaryOperation* nbinop = new BinaryOperation(nleft, noperation, nright);
        Number* nleft_is_number = dynamic_cast<Number*>(nleft);
        Number* nright_is_number = dynamic_cast<Number*>(nright);

        if (nleft_is_number && nright_is_number) {
            Expression* result = new Number(binop->evaluate());
            delete nbinop;
            return result;
        }
        else {
            return nbinop;
        }
    }

    Expression* transformFunctionCall(FunctionCall const* fcall)
    {
        Expression* arg = (fcall->arg())->transform(this);
        std::string const& nname = fcall->name();

        FunctionCall* nfcall = new FunctionCall(nname, arg);
        Number* arg_is_number = dynamic_cast<Number*>(arg);

        if (arg_is_number) {
            Expression* result = new Number(fcall->evaluate());
            delete nfcall;
            return result;
        }
        else {
            return nfcall;
        }
    }

    Expression* transformVariable(Variable const* var)
    {
        Expression* exp = new Variable(var->name());
        return exp;
    }

    ~FoldConstants() { };
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