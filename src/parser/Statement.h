#pragma once
#include <string>
#include <vector>

struct SingleStmt {
    std::string _code;
    // subint
    SingleStmt() = default;
    SingleStmt(std::string name, std::string lowerBound, std::string upperBound) {
        _code = "subint " + name + 
                " {" + lowerBound + ".." + 
                    upperBound + "};";
    }
    SingleStmt(std::string name, bool initialValue) {
        _code = "bool " + name +
        " = " + (initialValue ? "true" : "false") + ";";
    }
};

struct CompoundStmt {
    std::vector<std::string> _stmts;

    CompoundStmt() = default;

    CompoundStmt(SingleStmt stmt) {
        _stmts.push_back(stmt._code);
    }

    void addStmt(SingleStmt stmt) {
        _stmts.push_back(stmt._code);
    }

    void addStmts(CompoundStmt &stmts) {
        for (auto stmt: stmts._stmts) {
            _stmts.push_back(std::move(stmt));
        }
    }

    std::string to_string() {
        std::string res = "";
        for (auto stmt : _stmts) res += (stmt + "\n");
        return res;
    }
};

struct GuardStmt : SingleStmt{
    GuardStmt(std::string &expr) {
        _code = "[" + std::move(expr) + "] {}";
    }
    GuardStmt(std::string &expr, CompoundStmt stmt) {
        _code = "[" + std::move(expr) + "] " + stmt.to_string();
    }
};