/**
 * @file va2pa.cpp
 * @author Sina Karvandi (sina@hyperdbg.org)
 * @brief !va2pa command
 * @details
 * @version 0.1
 * @date 2020-07-16
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "..\hprdbgctrl\pch.h"

//
// Global Variables
//
extern BOOLEAN                  g_IsSerialConnectedToRemoteDebuggee;
extern ACTIVE_DEBUGGING_PROCESS g_ActiveProcessDebuggingState;

/**
 * @brief help of !va2pa command
 *
 * @return VOID
 */
VOID
CommandVa2paHelp()
{
    ShowMessages("!va2pa : Converts virtual address to physical address.\n\n");
    ShowMessages("syntax : \t!va2pa [VirtualAddress (hex)] [pid ProcessId (hex)]\n");
    ShowMessages("\t\te.g : !va2pa nt!ExAllocatePoolWithTag\n");
    ShowMessages("\t\te.g : !va2pa nt!ExAllocatePoolWithTag+5\n");
    ShowMessages("\t\te.g : !va2pa @rcx\n");
    ShowMessages("\t\te.g : !va2pa @rcx+5\n");
    ShowMessages("\t\te.g : !va2pa fffff801deadbeef\n");
    ShowMessages("\t\te.g : !va2pa fffff801deadbeef pid 0xc8\n");
}

/**
 * @brief !va2pa command handler
 *
 * @param SplittedCommand
 * @param Command
 * @return VOID
 */
VOID
CommandVa2pa(vector<string> SplittedCommand, string Command)
{
    BOOL                              Status;
    ULONG                             ReturnedLength;
    UINT64                            TargetVa;
    UINT32                            Pid            = 0;
    DEBUGGER_VA2PA_AND_PA2VA_COMMANDS AddressDetails = {0};
    vector<string>                    SplittedCommandCaseSensitive {Split(Command, ' ')};

    if (SplittedCommand.size() == 1 || SplittedCommand.size() >= 5 ||
        SplittedCommand.size() == 3)
    {
        ShowMessages("incorrect use of '!va2pa'\n\n");
        CommandVa2paHelp();
        return;
    }

    //
    // By default if the user-debugger is active, we use these commands
    // on the memory layout of the debuggee process
    //
    if (g_ActiveProcessDebuggingState.IsActive)
    {
        Pid = g_ActiveProcessDebuggingState.ProcessId;
    }

    if (SplittedCommand.size() == 2)
    {
        //
        // It's just an address for current process
        //
        if (!SymbolConvertNameOrExprToAddress(SplittedCommandCaseSensitive.at(1), &TargetVa))
        {
            //
            // Couldn't resolve or unkonwn parameter
            //
            ShowMessages("err, couldn't resolve error at '%s'\n\n",
                         SplittedCommandCaseSensitive.at(1).c_str());
            return;
        }
    }
    else
    {
        //
        // It might be address + pid
        //
        if (!SplittedCommand.at(1).compare("pid"))
        {
            if (!ConvertStringToUInt32(SplittedCommand.at(2), &Pid))
            {
                ShowMessages("incorrect address, please enter a valid process id\n\n");
                return;
            }

            if (!SymbolConvertNameOrExprToAddress(SplittedCommandCaseSensitive.at(3), &TargetVa))
            {
                //
                // Couldn't resolve or unkonwn parameter
                //
                ShowMessages("err, couldn't resolve error at '%s'\n\n",
                             SplittedCommandCaseSensitive.at(3).c_str());
                return;
            }
        }
        else if (!SplittedCommand.at(2).compare("pid"))
        {
            if (!SymbolConvertNameOrExprToAddress(SplittedCommandCaseSensitive.at(1), &TargetVa))
            {
                //
                // Couldn't resolve or unkonwn parameter
                //
                ShowMessages("err, couldn't resolve error at '%s'\n\n",
                             SplittedCommandCaseSensitive.at(1).c_str());
                return;
            }

            if (!ConvertStringToUInt32(SplittedCommand.at(3), &Pid))
            {
                ShowMessages("incorrect address, please enter a valid process id\n\n");
                return;
            }
        }
        else
        {
            ShowMessages("incorrect use of '!va2pa'\n\n");
            CommandVa2paHelp();
            return;
        }
    }

    if (!g_DeviceHandle)
    {
        ShowMessages("handle of the driver not found, probably the driver is not loaded. Did you "
                     "use 'load' command?\n");
        return;
    }

    //
    // Check to prevent using process id in !va2pa command
    //
    if (g_IsSerialConnectedToRemoteDebuggee && Pid != 0)
    {
        ShowMessages("err, you cannot specify 'pid' in the debugger mode\n\n");
        return;
    }

    if (Pid == 0)
    {
        Pid = GetCurrentProcessId();
    }

    //
    // Prepare the buffer
    // We use same buffer for input and output
    //
    AddressDetails.VirtualAddress     = TargetVa;
    AddressDetails.ProcessId          = Pid;
    AddressDetails.IsVirtual2Physical = TRUE;

    //
    // Send IOCTL
    //
    Status = DeviceIoControl(
        g_DeviceHandle,                           // Handle to device
        IOCTL_DEBUGGER_VA2PA_AND_PA2VA_COMMANDS,  // IO Control code
        &AddressDetails,                          // Input Buffer to driver.
        SIZEOF_DEBUGGER_VA2PA_AND_PA2VA_COMMANDS, // Input buffer length
        &AddressDetails,                          // Output Buffer from driver.
        SIZEOF_DEBUGGER_VA2PA_AND_PA2VA_COMMANDS, // Length of output
                                                  // buffer in bytes.
        &ReturnedLength,                          // Bytes placed in buffer.
        NULL                                      // synchronous call
    );

    if (!Status)
    {
        ShowMessages("ioctl failed with code 0x%x\n", GetLastError());
        return;
    }

    if (AddressDetails.KernelStatus == DEBUGGER_OPERATION_WAS_SUCCESSFULL)
    {
        //
        // Show the results
        //
        ShowMessages("%llx\n", AddressDetails.PhysicalAddress);
    }
    else
    {
        //
        // An err occurred, no results
        //
        ShowErrorMessage(AddressDetails.KernelStatus);
    }
}
