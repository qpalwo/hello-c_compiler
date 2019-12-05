#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include "util.h"
#include "label.h"
#include "symbol.h"
#include "absyn.h"
#include "codegen.h"

extern int CODEGEN_DEBUG;
int VERIFY = 0;
typedef struct CG_baseBlockList_ *CG_baseBlockList;
typedef struct CG_whileContext_ *CG_whileContext;

struct CG_whileContext_ {
    LLVMBasicBlockRef condBlock;
    LLVMBasicBlockRef endBlock;
};

LLVMBasicBlockRef CG_stmList(A_stmList stmList, LLVMModuleRef module, LLVMBuilderRef builder, CG_whileContext wcontext);

LLVMValueRef CG_exp(A_exp exp, LLVMModuleRef module, LLVMBuilderRef builder);

struct CG_baseBlockList_ {
    LLVMBasicBlockRef head;
    CG_baseBlockList tail;
};

CG_baseBlockList CG_BaseBlockList(LLVMBasicBlockRef head, CG_baseBlockList tail) {
    CG_baseBlockList list = checked_malloc(sizeof(*list));
    list->head = head;
    list->tail = tail;
    return list;
}

static CG_whileContext CG_WhileContext(LLVMBasicBlockRef cond, LLVMBasicBlockRef end) {
    CG_whileContext context = checked_malloc(sizeof(*context));
    context->condBlock = cond;
    context->endBlock = end;
    return context;
}

static void CG_dumpModule(LLVMModuleRef module) {
    if (CODEGEN_DEBUG) {
        LLVMDumpModule(module);
    }
}

static void InternalError(int pos, char *message, ...) {
    va_list ap;
    fprintf(stderr, "[Code Generation Error] line %d: ", pos);
    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

static LLVMValueRef unwrapIfAPointer(LLVMValueRef value, LLVMBuilderRef builder) {
    if (!value) {
        return NULL;
    }
    LLVMTypeRef type = LLVMTypeOf(value);
    LLVMTypeKind tykind = LLVMGetTypeKind(type);
    if (tykind != LLVMPointerTypeKind) {
        return value;
    }
    return LLVMBuildLoad(builder, value, Label_NewLabel("loaded"));
}

static void CG_initInternalFunction(LLVMModuleRef module, LLVMBuilderRef builder) {
    // declare printf
    LLVMTypeRef ret = LLVMInt32Type();
    LLVMTypeRef para[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef funty = LLVMFunctionType(ret, para, 1, TRUE);
    LLVMValueRef fun = LLVMAddFunction(module, "printf", funty);
    Label_NewDec(S_Symbol("printf"), fun);


    LLVMTypeRef intpara[] = { LLVMInt32Type() };
    LLVMTypeRef floatpara[] = { LLVMFloatType() };
    LLVMTypeRef charpara[] = { LLVMInt8Type() };

    LLVMTypeRef intfunty = LLVMFunctionType(LLVMVoidType(), intpara, 1, FALSE);
    LLVMTypeRef floatfunty = LLVMFunctionType(LLVMVoidType(), floatpara, 1, FALSE);
    LLVMTypeRef charfunty = LLVMFunctionType(LLVMVoidType(), charpara, 1, FALSE);

    LLVMValueRef intfun = LLVMAddFunction(module, "printi", intfunty);
    LLVMValueRef floatfun = LLVMAddFunction(module, "printfl", floatfunty);
    LLVMValueRef charfun = LLVMAddFunction(module, "printc", charfunty);

    Label_NewDec(S_Symbol("printi"), intfun);
    Label_NewDec(S_Symbol("printfl"), floatfun);
    Label_NewDec(S_Symbol("printc"), charfun);

    LLVMBasicBlockRef intblock = LLVMAppendBasicBlock(intfun, "<entry>");
    LLVMBasicBlockRef floatblock = LLVMAppendBasicBlock(floatfun, "<entry>");
    LLVMBasicBlockRef charblock = LLVMAppendBasicBlock(charfun, "<entry>");

    // build printi
    LLVMPositionBuilderAtEnd(builder, intblock);
    LLVMValueRef intStr = LLVMBuildGlobalStringPtr(builder, "int value: %d\n", "");
    LLVMValueRef argi[] = {
            intStr,
            LLVMGetParam(intfun, 0)
    };
    LLVMBuildCall(builder, Label_FindDec(S_Symbol("printf")), argi, 2, "");
    LLVMBuildRetVoid(builder);

    //build printc
    LLVMPositionBuilderAtEnd(builder, charblock);
    LLVMValueRef charStr = LLVMBuildGlobalStringPtr(builder, "char value: %c\n", "");
    LLVMValueRef argc[] = {
            charStr,
            LLVMGetParam(charfun, 0)
    };
    LLVMBuildCall(builder, Label_FindDec(S_Symbol("printf")), argc, 2, "");
    LLVMValueRef argtmp[] = {
            intStr,
            LLVMConstInt(LLVMInt32Type(), 10, FALSE)
    };
    LLVMBuildCall(builder, Label_FindDec(S_Symbol("printf")), argtmp, 2, "");
    LLVMBuildRetVoid(builder);

    // build printfl
    LLVMPositionBuilderAtEnd(builder, floatblock);
    LLVMValueRef floatStr = LLVMBuildGlobalStringPtr(builder, "float value: %f\n", "");
    LLVMValueRef argf[] = {
            floatStr,
            LLVMGetParam(floatfun, 0)
    };
    LLVMBuildCall(builder, Label_FindDec(S_Symbol("printf")), argf, 2, "");
    LLVMBuildRetVoid(builder);
}

static CG_table TYPE_TABLE;

void CG_typeInit() {
    TYPE_TABLE = S_NewTable(FALSE);
    S_Enter(TYPE_TABLE, S_Symbol("void"), LLVMVoidType());
    S_Enter(TYPE_TABLE, S_Symbol("int"), LLVMInt32Type());
    S_Enter(TYPE_TABLE, S_Symbol("float"), LLVMFloatType());
    S_Enter(TYPE_TABLE, S_Symbol("char"), LLVMInt32Type());
}

LLVMValueRef CG_ZeroValueOfType(LLVMTypeRef type) {
    switch (LLVMGetTypeKind(type)) {
        case LLVMIntegerTypeKind:
            return LLVMConstInt(LLVMInt32Type(), 0, FALSE);
        case LLVMFloatTypeKind:
            return LLVMConstReal(LLVMFloatType(), 0);
        case LLVMStructTypeKind:
        case LLVMArrayTypeKind:
        default:
            return NULL;
    }
}

LLVMTypeRef CG_dec2TypeRef(A_tyDec dec) {
    switch (dec->kind) {
        case A_VAR_DEC: {
            LLVMTypeRef dectype = S_Find(TYPE_TABLE, dec->u.var.type);
            if (!dectype) {
                InternalError(dec->linno, "no such type defined");
            }
            return dectype;
        }
        case A_ARRAY_DEC: {
            return NULL;
            // LLVMArrayType()
        }
        case A_NULL_DEC:
            return NULL;
    }
}

LLVMTypeRef *CG_decList2TypeRef(A_tyDecList declist, int *count) {
    A_tyDecList bodydec = declist;
    int bodyCount = 0;
    while (bodydec) {
        bodyCount++;
        bodydec = bodydec->tail;
    }
    *count = bodyCount;
    LLVMTypeRef *body = checked_malloc(sizeof(*body) * bodyCount);
    bodydec = declist;
    for (int i = 0; i < bodyCount; i++, bodydec = bodydec->tail) {
        body[i] = CG_dec2TypeRef(bodydec->head);
    }
    return body;
}

LLVMValueRef *CG_callExpList2Ref(A_expList expList, LLVMModuleRef module, LLVMBuilderRef builder, int *count) {
    int num = 0;
    A_expList list = expList;
    while (expList && expList->head) {
        num++;
        expList = expList->tail;
    }
    *count = num;
    LLVMValueRef *refList = checked_malloc(sizeof(*refList) * num);
    for (int i = 0; i < num && list; i++, list = list->tail) {
        refList[i] = CG_exp(list->head, module, builder);
    }
    return refList;
}

S_symbol TyDecName(A_tyDec dec) {
    switch (dec->kind) {
        case A_VAR_DEC: {
            return dec->u.var.name;
        }
        case A_ARRAY_DEC: {
            return dec->u.array.name;
        }
        case A_NULL_DEC:
            return NULL;
    }
}

LLVMValueRef CG_singleExp(A_exp exp, sop op, LLVMModuleRef module, LLVMBuilderRef builder) {
    LLVMValueRef expvalue = CG_exp(exp, module, builder);
    LLVMTypeRef exptype = LLVMTypeOf(expvalue);
    LLVMTypeKind exptypekind = LLVMGetTypeKind(exptype);
    switch (op) {
        case A_SPLUS: {
            if (exptypekind != LLVMPointerTypeKind) {
                InternalError(exp->linno, "alg exp type must be a pointer");
                return NULL;
            }
            LLVMTypeRef elementTy = LLVMGetElementType(exptype);
            LLVMTypeKind elementTyKind = LLVMGetTypeKind(elementTy);
            if (elementTyKind == LLVMIntegerTypeKind) {
                LLVMValueRef ONE = LLVMConstInt(LLVMInt32Type(), 1, FALSE);
                LLVMValueRef load = LLVMBuildLoad(builder, expvalue, Label_NewLabel("load"));
                LLVMValueRef added = LLVMBuildAdd(builder, load, ONE, Label_NewLabel("add"));
                return LLVMBuildStore(builder, added, expvalue);
            } else if (elementTyKind == LLVMFloatTypeKind) {
                LLVMValueRef ONE = LLVMConstReal(LLVMFloatType(), 1.0);
                LLVMValueRef load = LLVMBuildLoad(builder, expvalue, Label_NewLabel("load"));
                LLVMValueRef added = LLVMBuildFAdd(builder, expvalue, ONE, "");
                return LLVMBuildStore(builder, added, expvalue);
            } else {
                InternalError(exp->linno, "alg exp type must be float or int");
                return NULL;
            }
        }
        case A_SMINUS: {
            if (exptypekind != LLVMPointerTypeKind) {
                InternalError(exp->linno, "alg exp type must be a pointer");
                return NULL;
            }
            LLVMTypeRef elementTy = LLVMGetElementType(exptype);
            LLVMTypeKind elementTyKind = LLVMGetTypeKind(elementTy);
            if (elementTyKind == LLVMIntegerTypeKind) {
                LLVMValueRef ONE = LLVMConstInt(LLVMInt32Type(), -1, FALSE);
                LLVMValueRef load = LLVMBuildLoad(builder, expvalue, Label_NewLabel("load"));
                LLVMValueRef subed = LLVMBuildAdd(builder, load, ONE, Label_NewLabel("add"));
                return LLVMBuildStore(builder, subed, expvalue);
            } else if (elementTyKind == LLVMFloatTypeKind) {
                LLVMValueRef ONE = LLVMConstReal(LLVMFloatType(), -1.0);
                LLVMValueRef load = LLVMBuildLoad(builder, expvalue, Label_NewLabel("load"));
                LLVMValueRef subed = LLVMBuildFAdd(builder, expvalue, ONE, "");
                return LLVMBuildStore(builder, subed, expvalue);
            } else {
                InternalError(exp->linno, "alg exp type must be float or int");
                return NULL;
            }
        }
        case A_NEGATIVE: {
            return LLVMBuildNeg(builder, expvalue, "");
        }
        case A_POSITIVE: {
            return expvalue;
        }
        case A_NOT: {
            return LLVMBuildNot(builder, expvalue, "");
        }
    }

}

LLVMValueRef CG_doubleExp(A_exp le, A_exp re, dop op, LLVMModuleRef module, LLVMBuilderRef builder) {
    LLVMValueRef lexpvalue = unwrapIfAPointer(CG_exp(le, module, builder), builder);
    LLVMTypeRef lexptype = LLVMTypeOf(lexpvalue);
    LLVMTypeKind lexptypekind = LLVMGetTypeKind(lexptype);
    LLVMValueRef rexpvalue = unwrapIfAPointer(CG_exp(re, module, builder), builder);
    LLVMTypeRef rexptype = LLVMTypeOf(rexpvalue);
    LLVMTypeKind rexptypekind = LLVMGetTypeKind(rexptype);
    LLVMValueRef lfv = NULL, rfv = NULL;
    if (lexptypekind == LLVMFloatTypeKind || rexptypekind == LLVMFloatTypeKind) {
        if (lexptypekind == LLVMFloatTypeKind) {
            lfv = lexpvalue;
        } else {
            lfv = LLVMBuildIntCast2(builder, lexpvalue, LLVMFloatType(), TRUE, "");
        }
        if (rexptypekind == LLVMFloatTypeKind) {
            rfv = lexpvalue;
        } else {
            rfv = LLVMBuildIntCast2(builder, rexpvalue, LLVMFloatType(), TRUE, "");
        }
        assert(lfv && rfv);
    }
    LLVMIntPredicate compareOP = 0;
    LLVMRealPredicate compareFOP = 0;
    switch (op) {
        case A_PLUS: {
            if (!lfv) {
                return LLVMBuildAdd(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFAdd(builder, lfv, rfv, "");
            }
        }
        case A_MINUS: {
            if (!lfv) {
                return LLVMBuildSub(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFSub(builder, lfv, rfv, "");
            }
        }
        case A_TIMES: {
            if (!lfv) {
                return LLVMBuildMul(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFMul(builder, lfv, rfv, "");
            }
        }
        case A_DIVIDE: {
            if (!lfv) {
                return LLVMBuildSDiv(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFDiv(builder, lfv, rfv, "");
            }
        }
        case A_EQ:
            compareOP = LLVMIntEQ;
            compareFOP = LLVMRealOEQ;
            break;
        case A_NEQ:
            compareOP = LLVMIntNE;
            compareFOP = LLVMRealONE;
            break;
        case A_GT:
            compareOP = LLVMIntSGT;
            compareFOP = LLVMRealOGT;
            break;
        case A_GE:
            compareOP = LLVMIntSGE;
            compareFOP = LLVMRealOGE;
            break;
        case A_LT:
            compareOP = LLVMIntSLT;
            compareFOP = LLVMRealOLT;
            break;
        case A_LE:
            compareOP = LLVMIntSLE;
            compareFOP = LLVMRealOLE;
            break;
        case A_BITAND: {
            if (!lfv) {
                InternalError(le->linno, "logic algo must be int");
                return LLVMConstInt(LLVMInt32Type(), 0, FALSE);
            } else {
                return LLVMBuildAnd(builder, lfv, rfv, "");
            }
        }
        case A_BITOR: {
            if (!lfv) {
                InternalError(le->linno, "logic algo must be int");
                return LLVMConstInt(LLVMInt32Type(), 0, FALSE);
            } else {
                return LLVMBuildOr(builder, lfv, rfv, "");
            }
        }
        case A_AND:
        case A_OR:;
    }
    if (lfv) {
        return LLVMBuildFCmp(builder, compareFOP, lfv, rfv, "");
    } else {
        return LLVMBuildICmp(builder, compareOP, lexpvalue, rexpvalue, "");
    }
}

LLVMValueRef CG_var(A_var var, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch (var->kind) {
        case A_SYMBOL_VAR: {
            return Label_FindDec(var->u.symbol);
        }
        case A_ARRAY_VAR: {

        }
        case A_STRUCT_VAR: {

        }
    }

}

LLVMValueRef CG_exp(A_exp exp, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch (exp->kind) {
        case A_CONST: {
            switch (exp->u.cons.kind) {
                case A_INT:
                    return LLVMConstInt(LLVMInt32Type(), exp->u.cons.u.inum, FALSE);
                case A_CHAR:
                    return LLVMConstInt(LLVMInt32Type(), exp->u.cons.u.cnum, FALSE);
                case A_FLOAT:
                    return LLVMConstReal(LLVMFloatType(), exp->u.cons.u.fnum);
                case A_VAR:
                    return CG_var(exp->u.cons.u.var, module, builder);
            }
        }
        case A_CALL: {
            LLVMValueRef fun = Label_FindDec(exp->u.call.name);
            if (!fun) {
                InternalError(exp->linno, "find call fun error");
                return NULL;
            }
            int paraCount = 0;
            LLVMValueRef *paras = CG_callExpList2Ref(exp->u.call.para, module, builder, &paraCount);
            LLVMValueRef call = LLVMBuildCall(builder, fun, paras, paraCount, "");
            if (!call) {
                InternalError(exp->linno, "create call instrction error");
                return NULL;
            }
            return call;
        }
        case A_DOUBLE_EXP: {
            return CG_doubleExp(
                    exp->u.doublexp.left, exp->u.doublexp.right, exp->u.doublexp.op, module, builder);
        }
        case A_SINGLE_EXP: {
            return CG_singleExp(exp->u.singexp.exp, exp->u.singexp.op, module, builder);
        }
    }
}

LLVMBasicBlockRef CG_stm(A_stm stm, LLVMModuleRef module, LLVMBuilderRef builder, CG_whileContext wcontext) {
    switch (stm->kind) {
        case A_ASSIGN_STM: {
            LLVMValueRef exp = CG_exp(stm->u.assign.exp, module, builder);
            if (!exp) {
                InternalError(stm->linno, "create assign stm exp error");
            }
            LLVMValueRef value = CG_var(stm->u.assign.symbol, module, builder);
            if (!value) {
                InternalError(stm->linno, "get assign stm value error");
            }
            LLVMBuildStore(builder, exp, value);
            return NULL;
        }
        case A_DEC_STM: {
            LLVMTypeRef dectype = CG_dec2TypeRef(stm->u.dec);
            if (!dectype) {
                InternalError(stm->linno, "create dec error");
            }
            S_symbol name = TyDecName(stm->u.dec);
            LLVMValueRef dec = LLVMBuildAlloca(builder, dectype, S_Name(name));
            Label_NewDec(name, dec);
            return NULL;
        }
        case A_IF_STM: {
            LLVMBasicBlockRef nowBlock = LLVMGetInsertBlock(builder);
            LLVMValueRef fun = LLVMGetBasicBlockParent(nowBlock);
            LLVMValueRef cond = unwrapIfAPointer(CG_exp(stm->u.iff.test, module, builder), builder);
            LLVMValueRef INTZERO = LLVMConstInt(LLVMInt32Type(), 0, FALSE);
            cond = LLVMBuildICmp(builder, LLVMIntNE, cond, INTZERO, Label_NewLabel("ifcond"));
            LLVMBasicBlockRef then = LLVMAppendBasicBlock(fun, Label_NewLabel("if.then"));
            LLVMBasicBlockRef elsee = LLVMAppendBasicBlock(fun, Label_NewLabel("if.else"));
            LLVMBasicBlockRef end = LLVMAppendBasicBlock(fun, Label_NewLabel("if.end"));
            LLVMPositionBuilderAtEnd(builder, nowBlock);
            LLVMValueRef condins = LLVMBuildCondBr(builder, cond, then, elsee);
            LLVMPositionBuilderAtEnd(builder, then);
            Label_NewScope();
            LLVMBasicBlockRef thenEndBlock = CG_stmList(stm->u.iff.iff, module, builder, wcontext);
            if (!thenEndBlock) {
                LLVMBuildBr(builder, end);
            }
            Label_EndScope();
            LLVMPositionBuilderAtEnd(builder, elsee);
            Label_NewScope();
            LLVMBasicBlockRef elseEndBlock = CG_stmList(stm->u.iff.elsee, module, builder, wcontext);
            if (!elseEndBlock) {
                LLVMBuildBr(builder, end);
            }
            Label_EndScope();
            // handle nested end block
            if (thenEndBlock) {
                LLVMPositionBuilderAtEnd(builder, thenEndBlock);
                if (!LLVMGetBasicBlockTerminator(thenEndBlock)) {
                    LLVMBuildBr(builder, end);
                }
            }
            if (elseEndBlock) {
                LLVMPositionBuilderAtEnd(builder, elseEndBlock);
                if (!LLVMGetBasicBlockTerminator(elseEndBlock)) {
                    LLVMBuildBr(builder, end);
                }
            }
            // set builder pointer at next BaseBlock
            LLVMPositionBuilderAtEnd(builder, end);
            return end;
        }
        case A_EXP_STM: {
            CG_exp(stm->u.exp, module, builder);
            return NULL;
        }
        case A_WHILE_STM: {
            LLVMBasicBlockRef nowBlock = LLVMGetInsertBlock(builder);
            LLVMValueRef fun = LLVMGetBasicBlockParent(nowBlock);
            LLVMBasicBlockRef condBlock = LLVMAppendBasicBlock(fun, Label_NewLabel("while.cond"));
            LLVMBasicBlockRef bodyBlock = LLVMAppendBasicBlock(fun, Label_NewLabel("while.body"));
            LLVMBasicBlockRef endBlock = LLVMAppendBasicBlock(fun, Label_NewLabel("while.end"));
            // create now while context
            CG_whileContext mWContext = CG_WhileContext(condBlock, endBlock);
            // build Br to condBlock
            LLVMPositionBuilderAtEnd(builder, nowBlock);
            LLVMBuildBr(builder, condBlock);
            // build condBlock
            LLVMPositionBuilderAtEnd(builder, condBlock);
            LLVMValueRef cond = unwrapIfAPointer(CG_exp(stm->u.whilee.test, module, builder), builder);
            LLVMValueRef INTZERO = LLVMConstInt(LLVMInt32Type(), 0, FALSE);
            cond = LLVMBuildICmp(builder, LLVMIntNE, cond, INTZERO, Label_NewLabel("whilecond"));
            LLVMBuildCondBr(builder, cond, bodyBlock, endBlock);
            // build while body
            LLVMPositionBuilderAtEnd(builder, bodyBlock);
            Label_NewScope();
            LLVMBasicBlockRef bodyend = CG_stmList(stm->u.whilee.whilee, module, builder, mWContext);
            Label_EndScope();
            LLVMBuildBr(builder, condBlock);
            // handle body end block 
            LLVMPositionBuilderAtEnd(builder, bodyend);
            LLVMBuildBr(builder, endBlock);
            // merge while block
            LLVMPositionBuilderAtEnd(builder, endBlock);
            return endBlock;
        }
        case A_BREAK_STM: {
            if (!wcontext) {
                InternalError(stm->linno, "break not in a while block");
            }
            LLVMBuildBr(builder, wcontext->endBlock);
            return NULL;
        }
        case A_CONTINUE_STM: {
            if (!wcontext) {
                InternalError(stm->linno, "break not in a while block");
            }
            LLVMBuildBr(builder, wcontext->condBlock);
            return NULL;
        }
        case A_RETURN_STM: {
            LLVMValueRef retval = unwrapIfAPointer(CG_exp(stm->u.returnn, module, builder), builder);
            if (retval) { 
                LLVMBuildRet(builder, retval);
            } else {
                LLVMBuildRetVoid(builder);
            }
            return NULL;
        }
    }
}

LLVMBasicBlockRef CG_stmList(A_stmList stmList, LLVMModuleRef module, LLVMBuilderRef builder, CG_whileContext wcontext) {
    CG_baseBlockList endblock = NULL;
    while (stmList && stmList->head) {
        LLVMBasicBlockRef end = CG_stm(stmList->head, module, builder, wcontext);
        if (end) {
            endblock = CG_BaseBlockList(end, endblock);
        }
        stmList = stmList->tail;
    }
    // merge end block
    if (endblock && endblock->head) {
        LLVMValueRef fun = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
        LLVMBasicBlockRef endbb = LLVMAppendBasicBlock(fun, Label_NewLabel("end"));
        while (endblock && endblock->head) {
            LLVMPositionBuilderAtEnd(builder, endblock->head);
            if (!LLVMGetBasicBlockTerminator(endblock->head)) {
                LLVMBuildBr(builder, endbb);
            }
            endblock = endblock->tail;
        }
        return endbb;
    } else {
        return NULL;
    }
}

void CG_globalDec(A_globalDec globaldec, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch (globaldec->kind) {
        case A_FUN: {
            LLVMTypeRef rettype = S_Find(TYPE_TABLE, globaldec->u.fun.ret);
            if (!rettype) {
                InternalError(globaldec->linno, "create fun error");
                return;
            }
            int paraCount = 0;
            LLVMTypeRef *para = CG_decList2TypeRef(globaldec->u.fun.para, &paraCount);
            LLVMTypeRef funty = LLVMFunctionType(rettype, para, paraCount, FALSE);
            if (!funty) {
                InternalError(globaldec->linno, "create fun error");
                return;
            }
            LLVMValueRef fun = LLVMAddFunction(module, S_Name(globaldec->u.fun.name), funty);
            if (!fun) {
                InternalError(globaldec->linno, "create fun error");
                return;
            }

            LLVMBasicBlockRef block = LLVMAppendBasicBlock(fun, Label_NewFun(globaldec->u.fun.name, fun));
            LLVMPositionBuilderAtEnd(builder, block);
            // alloc para
            A_tyDecList paralist = globaldec->u.fun.para;
            LLVMTypeRef *typeList = checked_malloc(sizeof(*typeList) * paraCount);
            LLVMGetParamTypes(funty, typeList);
            for (int i = 0; i < paraCount && paralist; i++, paralist = paralist->tail) {
                S_symbol decName = TyDecName(paralist->head);
                LLVMValueRef paravalue = LLVMBuildAlloca(builder, typeList[i], S_Name(decName));
                LLVMBuildStore(builder, LLVMGetParam(fun, i), paravalue);
                Label_NewDec(decName, paravalue);
            }
            // build fun body
            LLVMBasicBlockRef bodyend = CG_stmList(globaldec->u.fun.body, module, builder, NULL);
            if (bodyend) {
                LLVMPositionBuilderAtEnd(builder, bodyend);
            }
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMValueRef retv = CG_ZeroValueOfType(rettype);
                if (retv) {
                    LLVMBuildRet(builder, retv);
                } else {
                    LLVMBuildRetVoid(builder);
                }
            }
            Label_EndFun();
            if (VERIFY && LLVMVerifyFunction(fun, LLVMPrintMessageAction) == 1) {
                InternalError(globaldec->linno, "error function");
                LLVMDeleteFunction(fun);
            }
            break;
        }
        case A_STRUCT: {
            LLVMTypeRef struc = LLVMStructCreateNamed(LLVMGetModuleContext(module), S_Name(globaldec->u.struc.name));
            if (!struc) {
                InternalError(globaldec->linno, "create type error");
            }
            A_tyDecList bodydec = globaldec->u.struc.declist;
            int bodyCount = 0;
            LLVMTypeRef *body = CG_decList2TypeRef(globaldec->u.struc.declist, &bodyCount);
            LLVMStructSetBody(struc, body, bodyCount, FALSE);
            S_Enter(TYPE_TABLE, globaldec->u.struc.name, struc);
            break;
        }
        case A_GLOBAL_VAR: {
            LLVMTypeRef dectype = S_Find(TYPE_TABLE, globaldec->u.var.type);
            if (!dectype) {
                InternalError(globaldec->linno, "no such type defined");
            }
            LLVMValueRef globalvar = LLVMAddGlobal(module, dectype, S_Name(globaldec->u.var.name));
            if (!globalvar) {
                InternalError(globaldec->linno, "var define error");
            }
            break;
        }
    }
}

void execute(LLVMModuleRef module, LLVMValueRef func) {
    string error = NULL;
    LLVMExecutionEngineRef engine;

    LLVMLinkInInterpreter();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    if (LLVMCreateExecutionEngineForModule(&engine, module, &error) != 0) {
        fprintf(stderr, "failed to create execution engine\n");
        abort();
    }
    if (error) {
        fprintf(stderr, "error: %s\n", error);
        LLVMDisposeMessage(error);
        exit(-1);
    }
    LLVMGenericValueRef args[] = {
        LLVMCreateGenericValueOfInt(LLVMInt32Type(), 5, 0),
        LLVMCreateGenericValueOfInt(LLVMInt32Type(), 10, 0)
    };
    LLVMGenericValueRef res = LLVMRunFunction(engine, func, 2, args);
    printf("%d\n", (int)LLVMGenericValueToInt(res, 0));
    LLVMDisposeExecutionEngine(engine);
}

void CG_codeGen(A_globalDecList ast) {
    Label_InitTable();
    LLVMModuleRef module = LLVMModuleCreateWithName("main-module");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    CG_typeInit();

    CG_initInternalFunction(module, builder);
    while (ast && ast->head) {
        CG_globalDec(ast->head, module, builder);
        ast = ast->tail;
    }
    string error = NULL;
    if (VERIFY) {
        LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
        if (!error) {
            InternalError(-1, error);
            return;
        }
    }
    CG_dumpModule(module);
    LLVMValueRef exfun = Label_FindDec(S_Symbol("main"));
    if (exfun) {
        execute(module, exfun);
    }

    LLVMDisposeBuilder(builder);
//    LLVMDisposeModule(module);
}