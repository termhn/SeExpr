/*
 Copyright Disney Enterprises, Inc.  All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License
 and the following modification to it: Section 6 Trademarks.
 deleted and replaced with:

 6. Trademarks. This License does not grant permission to use the
 trade names, trademarks, service marks, or product names of the
 Licensor and its affiliates, except as required for reproducing
 the content of the NOTICE file.

 You may obtain a copy of the License at
 http://www.apache.org/licenses/LICENSE-2.0
*/
#ifndef ExprParser_h
#define ExprParser_h

#ifndef MAKEDEPEND
#include <string>
#endif

namespace SeExpr2 {
class ExprNode;
class Expression;
bool ExprParse(SeExpr2::ExprNode*& parseTree,
               std::string& error,
               int& errorStart,
               int& errorEnd,
               std::vector<std::pair<int, int> >& _comments,
               const SeExpr2::Expression* expr,
               const char* str,
               bool wantVec = true);
}

#endif
