/*
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See the LICENSE file in the project root for full license information.
 */

#include "stdafx.h"
#include "Tpm2.h"
#include "Samples.h"

//added for main parameter parse
#include <iostream>
#include <cstring>
#include <cstdlib>  // for strtoull
#include <string>

#if __linux__
#define TCHAR   char  
#define _tcscmp strcmp
#define _T(s)   s
#endif

using namespace std;
using namespace TpmCpp;

bool UseSimulator = false;

static bool
CmdLine_IsOpt(
    const TCHAR* opt,               // Command line parameter to check
    const TCHAR* optFull,           // Expected full name
    const TCHAR* optShort = nullptr // Expected short (single letter) name
    )
{
    return 0 == wcscmp(opt, optFull)
        || (   (opt[0] == '/' || opt[0] == '-')
            && (   0 == _tcscmp(opt + 1, optFull)
                || (optShort && opt[1] == optShort[0] && opt[2] == 0)
                || (opt[0] == '-' && opt[1] == '-' && 0 == _tcscmp(opt + 2, optFull))));
}

void CmdLine_Help(ostream& ostr)
{
    ostr << "One command line option can be specified." << endl
        << "An option can be in the short form (one letter preceded with '-' or '/')" << endl
        << "or in the full form (preceded with '--' or without any sigil)." << endl
        << "Supported options:" << endl
        << "   -h (help|?) - print this message" << endl
        << "   -s (sim) - use locally running TPM simulator" << endl
        << "   -t (tbs|sys) - use system TPM" << endl;
}

int CmdLine_Parse(int argc, TCHAR *argv[])
{
    if (argc > 2)
    {
        cerr << "Too many command line option can be specified." << endl;
        CmdLine_Help(cerr);
        return -1;
    }
    if (argc == 1 || CmdLine_IsOpt(argv[1], _T("sim"), _T("s")))
    {
        UseSimulator = true;
        return 0;
    }
    if (CmdLine_IsOpt(argv[1], _T("tbs"), _T("t")) ||
        CmdLine_IsOpt(argv[1], _T("sys")))
    {
        UseSimulator = false;
        return 0;
    }
    if (CmdLine_IsOpt(argv[1], _T("help"), _T("h")) ||
        CmdLine_IsOpt(argv[1], _T("?"), _T("?")))
    {
        CmdLine_Help(cout);
        return 1;
    }

    cerr << "Unrecognized command line option: '" << argv[1] << "'" << endl;
    CmdLine_Help(cerr);
    return -2;
}

enum Operation { CREATE_COUNTER, DELETE_COUNTER, READ_COUNTER, INCREASE_COUNTER };
Operation g_operation;
int g_iIndex = 0;
std::string g_strPassword;

bool parseArguments(int argc, char* argv[]) {
    //if (argc < 5) {  // 现在需要至少5个参数 --index, value, --operation, value, -password, value
    if (argc < 7) {  // 现在需要至少5个参数 --index, value, --operation, value, --password, value
        std::cerr << "Usage: tpmcmd.exe --index <hex_value> --operation [create|delete|read|increase] --password <password>\n";
        return false;
    }

    bool indexFound = false;
    bool operationFound = false;
    bool passwordFound = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--index") == 0 && i + 1 < argc) {
            g_iIndex = strtoull(argv[i + 1], nullptr, 16);
            indexFound = true;
            ++i; // skip the value
        }
        else if (strcmp(argv[i], "--operation") == 0 && i + 1 < argc) {
            char* op = argv[i + 1];
            if (strcmp(op, "create") == 0) {
                g_operation = CREATE_COUNTER;
            }
            else if (strcmp(op, "delete") == 0) {
                g_operation = DELETE_COUNTER;
            }
            else if (strcmp(op, "read") == 0) {
                g_operation = READ_COUNTER;
            }
            else if (strcmp(op, "increase") == 0) {
                g_operation = INCREASE_COUNTER;
            }
            else {
                std::cerr << "Invalid operation: " << op << "\n";
                return false;
            }
            operationFound = true;
            ++i;
        }
        else if (strcmp(argv[i], "--password") == 0 && i + 1 < argc) {
            g_strPassword = argv[i + 1];
            passwordFound = true;
            ++i;
        }
        else {
            std::cerr << "Unknown argument or missing value: " << argv[i] << "\n";
            return false;
        }
    }

    if (!indexFound) {
        std::cerr << "--index argument missing\n";
        return false;
    }
    if (!operationFound) {
        std::cerr << "--operation argument missing\n";
        return false;
    }
    if (!passwordFound) {
        std::cerr << "--password argument missing\n";
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    int iRet = 1;  //actually exit value

    if (!parseArguments(argc, argv)) {
        return 1;
    }
    std::cout << "Parsed index: 0x" << std::hex << g_iIndex << std::dec << "\n";
    std::cout << "Parsed operation: " << g_operation << std::endl;
    std::cout << "Parsed password: " << g_strPassword << "\n";

    Samples s;
    switch (g_operation) {
    case CREATE_COUNTER: 
    {
        std::cout << "CREATE_COUNTER" << std::endl;
        if (s.CreateCounter(g_iIndex, g_strPassword) != 1) { //function return 1 means success
            return 1;                                        //main function return 1 means fail(like exit 1)
        }
    }
    break;

    case DELETE_COUNTER: 
    {
        std::cout << "DELETE_COUNTER" << std::endl; 
        if (s.DeleteCounter(g_iIndex, g_strPassword) != 1) {
            return 1;
        }
    }
    break;

    case READ_COUNTER: 
    {
        std::cout << "READ_COUNTER" << std::endl;
        int64_t iCounterVal = s.ReadCounter(g_iIndex, g_strPassword);
        std::cout << "read couter return value(decimal):" << iCounterVal << std::endl;
        if (iCounterVal < 0) {
            return 1;
        }
    }
    break;

    case INCREASE_COUNTER: 
    { 
        std::cout << "INCREASE_COUNTER" << std::endl;        
        if (s.IncreaseCounter(g_iIndex, g_strPassword) != 1) {
            return 1;
        }
    } 
    break;

    default: 
        std::cout << "UNKNOWN"; break;
    }
    
    return 0;
}
