//===- svf-ex.cpp -- A driver example of SVF-------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013->  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===-----------------------------------------------------------------------===//

/*
 // A driver program of SVF including usages of SVF APIs
 //
 // Author: Yulei Sui,
 */

#include "Graphs/IRGraph.h"
#include "SVF-LLVM/BasicTypes.h"
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/LLVMModule.h"
#include "Graphs/SVFG.h"
#include "SVFIR/SVFValue.h"
#include "WPA/Andersen.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "SVF-LLVM/BasicTypes.h"
#include "Util/Options.h"
#include "WPA/FlowSensitive.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>
#include "WPA/Andersen.h"
#include "FastCluster/fastcluster.h"
#include "Graphs/SVFGOPT.h"
#include "MemoryModel/PointerAnalysisImpl.h"
#include "MSSA/SVFGBuilder.h"
#include "WPA/WPAFSSolver.h"

using namespace llvm;
using namespace std;
using namespace SVF;

/*!
 * An example to query alias results of two LLVM values
 */
SVF::AliasResult aliasQuery(PointerAnalysis* pta, Value* v1, Value* v2)
{
    SVFValue* val1 = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v1);
    SVFValue* val2 = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v2);

    return pta->alias(val1,val2);
}

/*!
 * An example to print points-to set of an LLVM value
 */
std::string printPts(PointerAnalysis* pta, Value* val)
{

    std::string str;
    raw_string_ostream rawstr(str);
    SVFValue* svfval = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(val);


    NodeID pNodeId = pta->getPAG()->getValueNode(svfval);

    errs() << pNodeId  <<" " << svfval->toString() << "\n";
    const PointsTo& pts = pta->getPts(pNodeId);
    for (PointsTo::iterator ii = pts.begin(), ie = pts.end();
            ii != ie; ii++)
    {
        errs() << "\t " << *ii << " ";
        PAGNode* targetObj = pta->getPAG()->getGNode(*ii);
        if(targetObj->hasValue())
        {   
            errs() << "(" << targetObj->getValue()->toString() << ")\t ";
        }
        errs()<<"\n";
    }

    return rawstr.str();

}


int main(int argc, char ** argv)
{

    std::vector<std::string> moduleNameVec;
    //svf的一些命令行设置，不重要
    moduleNameVec = OptionBase::parseOptions(
            argc, argv, "Whole Program Points-to Analysis", "[options] <input-bitcode...>"
    );
    if (Options::WriteAnder() == "ir_annotator")
    {
        LLVMModuleSet::preProcessBCs(moduleNameVec);
    }
    //读取ir文件并转换为svfmodule
    SVFModule* svfModule = LLVMModuleSet::buildSVFModule(moduleNameVec);

    //获取llvm module
    Module* llvmModule = LLVMModuleSet::getLLVMModuleSet()->getMainLLVMModule();
    /// Build Program Assignment Graph (SVFIR)
    SVFIRBuilder builder(svfModule);
    /// 构建pag
    SVFIR* pag = builder.build();
    Andersen* ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    FlowSensitive* point_to_analysis = FlowSensitive::createFSWPA(pag);
    point_to_analysis->analyze();

    std::vector<llvm::Value*> values;
    // 遍历模块中的所有函数
    for (auto& function : llvmModule->getFunctionList()) {
        // 遍历函数中的所有基本块
        for (auto& basicBlock : function) {
            // 遍历基本块中的所有指令
            for (auto& instruction : basicBlock) {
                // 将指令本身加入向量
                values.push_back(&instruction);

                // 遍历指令的操作数
                for (auto& operand : instruction.operands()) {
                    values.push_back(operand);
                }
            }
        }
    }
    for(auto llvmvalue: values) {
        printPts(point_to_analysis,llvmvalue);
    }
    return 0;
}

