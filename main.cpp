#include <list>     // list
#include <map>      // map
#include <string>   // string, stod
#include <iostream> // cout
#include <math.h>   // pow

// ""s
using namespace std::string_literals;

// Funciones para validar caracteres
#define IsMul(curr) (curr == '*' || curr == '/' || curr == '%')
#define IsAdd(curr) (curr == '+' || curr == '-')
#define IsPow(curr) (curr == 'p')
#define IsNum(curr) (('0' <= curr && curr <= '9') || curr == '.')
#define HasSign(curr) (curr == '+' || curr == '-')
#define curr expresion[i]

// Estructura de arbol de n elementos
struct Node
{
    std::string value;
    std::list<Node> children = {};

    constexpr bool empty() const
    {
        return value == "";
    }

    Node &add(Node const &toAdd)
    {
        if (!toAdd.empty())
            children.push_back(toAdd);
        return *this;
    }

    void print(int deep = 0);
};

// Gramatica para validación de expresiones aritmeticas
// La funcion inicial es: Expr()
struct Grammat
{
    std::string expresion;
    int i = 0;

    Node Expr();
    Node ExprAux();
    Node Term();
    Node TermAux();
    Node Factor();
    Node FactorAux();
    Node Base();

    Node Number();
};

// Evalua un arbol
double eval(Node const &root);
// Valida y evalua una expresion aritmetica
double eval(std::string const &expresion, bool print_st = false);

void help()
{
    std::cout << "\n\t[Usage]: program [options] <expression>\n";
    std::cout << "\t[options]: -t                      Imprime el arbol sintactico\n";
    std::cout << "\t           -h                      Imprime la ayuda\n";
}

// Programa
int main(int argc, char const *argv[])
{
    bool print_syntax_tree = false;

    if (argc == 1)
    {
        help();
        return 0;
    }

    for (int i = 1; i < argc; ++i)
    {
        std::string option = argv[i];
        if (option == "-t") print_syntax_tree = true;
        if (option == "-h") { help(); return 0;  }
    }

    std::cout << eval(argv[argc - 1], print_syntax_tree) << '\n';
    return 0;
}

void test()
{
    std::cout << "2+2=" << eval("2+2,4") << '\n';
    std::cout << "6*7/3+2=" << eval("6*7/3+2") << '\n';
    std::cout << "10p3=" << eval("10p3") << '\n';
    std::cout << "34+(4/0.85)+*3=" << eval("34+(4/0.85)+*3") << '\n';
}

// <Expr> -> <Term> <ExprAux>
Node Grammat::Expr()
{
    Node node = Node({"+"});
    node.add(Term());
    node.add(ExprAux());
    return node;
}

// <ExprAux> -> + <Term> <ExprAux> | - <Term> <ExprAux> | E
Node Grammat::ExprAux()
{
    if (!IsAdd(curr))
        return {};

    Node node = Node({"+"});

    node.add(Term());
    node.add(ExprAux());
    return node;
}

// <Term> -> <Factor> <TermAux>
Node Grammat::Term()
{
    Node node = Node({"*"});
    node.add(Factor());
    node.add(TermAux());
    return node;
}

// <TermAux> -> * <Factor> <TermAux>
// <TermAux> -> / <Factor> <TermAux> | E
Node Grammat::TermAux()
{
    if (!IsMul(curr))
        return {};

    Node node = {curr + ""s};
    i++;

    node.add(Factor());
    node.add(TermAux());
    return node;
}

// <Factor> -> <Base> <FactorAux>
Node Grammat::Factor()
{
    Node node = Node({"p"});
    node.add(Base());
    node.add(FactorAux());
    return node;
}

// <FactorAux> -> p <Base> <FactorAux> | E
Node Grammat::FactorAux()
{
    if (!IsPow(curr))
        return {};
    i++;
    Node node = {"p"};
    node.add(Base());
    node.add(FactorAux());
    return node;
}

// <Base> -> <Expr> | <Number>
Node Grammat::Base()
{
    Node node = {"+"};

    if (HasSign(curr))
    {
        node = {curr + ""s};
        ++i;
    }

    if (IsNum(curr))
    {
        node.add(Number());
        return node;
    }
    if (curr == '(')
    {
        i++;
        node.add(Expr());
        if (curr != ')')
            throw "Se esperaba cierre de parentesis"s;
        i++;
        return node;
    }
    throw "Se esperaba un termino"s;
}

// evalua las expresiones numericas usando un automata finito determinista
Node Grammat::Number()
{
    static const std::map<std::string, std::string> alphabets = {
        {"number", "0123456789"},
        {"point", "."},
        {"signs", "+-"},
        {"exp", "Ee"},
    };

    static const std::map<std::string, std::map<std::string, std::string>> automat = {
        {"Init", {{"number", "Digit"}, {"point", "Point"}, {"signs", "Sign"}}},
        {"Sign", {{"number", "Digit"}, {"point", "Point"}}},
        {"Digit", {{"number", "Digit"}, {"point", "DSDP"}, {"exp", "ED"}}},
        {"DSDP", {{"number", "DSDP"}, {"exp", "ED"}}},
        {"ED", {{"number", "DED"}, {"signs", "DES"}}},
        {"DED", {{"number", "DED"}}},
        {"DES", {{"number", "DED"}}},
        {"Point", {{"number", "DSDP"}}},
    };

    static const std::map<std::string, bool> accepts = {
        {"Digit", true},
        {"DSDP", true},
        {"DED", true},
    };

    Node node;

    std::string state = "Init";
    std::string value = "";
    bool end = false;
    int initial_i = i;

    while (!end)
    {
        end = true;
        for (auto [name, content] : alphabets)
        {
            if (!content.contains(curr))
                continue;
            end = false;

            try
            {
                state = automat.at(state).at(name);
            }
            catch (std::out_of_range &)
            {
                end = true;
                break;
            }

            value.push_back(curr);
            i++;
        }
    }

    if (accepts.find(state) == accepts.end())
    {
        throw "Formato de numero invalido"s;
    }

    node.value = value;

    return node;
}

// Imprime el arbol
void Node::print(int deep)
{
    for (int i = 0; i < deep; ++i)
        std::cout << "> ";
    std::cout << value << '\n';
    for (auto child : children)
        child.print(deep + 1);
}

double eval(Node const &root)
{
    double res = 0.0;
    if (root.value == "+")
    {
        for (auto &child : root.children)
            res += eval(child);
    }
    else if (root.value == "-")
    {
        for (auto &child : root.children)
            res -= eval(child);
    }
    else if (root.value == "*")
    {
        res = 1.0;
        for (auto &child : root.children)
            res *= eval(child);
    }
    else if (root.value == "/")
    {
        if (root.children.size() == 2)
        {
            double num = eval(root.children.front());
            double denom = eval(root.children.back());
            
            if (denom == 0.0)
                std::cout << "[ERROR]: Division por cero\n";
            
            res = num / denom;
        }
        if (root.children.size() == 1)
        {
            double denom = eval(root.children.front());

            if (denom == 0.0)
                std::cout << "[ERROR]: Division por cero\n";
                
            res = 1.0 / denom;
        }
    }
    else if (root.value == "p")
    {
        if (root.children.size() == 2)
        {
            res = std::pow(
                eval(root.children.front()),
                eval(root.children.back()));
        }
        if (root.children.size() == 1)
        {
            res = eval(root.children.front());
        }
    }
    else if (IsNum(root.value[0]))
    {
        res = std::stod(root.value);
    }
    return res;
}

void print_error(std::string const& error, Grammat const& grammat)
{
    std::cout << "\n[ERROR]: [" << error << "]\n";
    std::cout << "[EN]: " << grammat.expresion << '\n';
    std::cout << "      ";
    for (int i = 0; i < grammat.i; ++i)
        std::cout << " ";
    std::cout << "^\n";
}

double eval(std::string const &expresion, bool print_st)
{
    Grammat grammat;

    grammat.expresion = expresion;

    Node arbol;
    try
    {
        arbol = grammat.Expr();
    }
    catch (std::string const &error)
    {
        print_error(error, grammat);
    }

    if (grammat.i != grammat.expresion.size())
    {
        print_error("No se esperaba el simbolo '"s + grammat.expresion[grammat.i] + '\'', grammat);
    }

    if (print_st)
    {
        arbol.print();
    }

    return eval(arbol);
}
