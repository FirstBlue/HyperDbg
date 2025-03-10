/**
 * @file IdtEmulation.h
 * @author Sina Karvandi (sina@hyperdbg.org)
 * @brief Header for Handlers of Guest's IDT Emulator
 * @details
 * @version 0.1
 * @date 2020-06-10
 * 
 * @copyright This project is released under the GNU Public License v3.
 * 
 */
#pragma once

//////////////////////////////////////////////////
//				     Structures		      		//
//////////////////////////////////////////////////

/**
 * @brief Guest interruptibility state
 * 
 */
typedef struct _INTERRUPTIBILITY_STATE
{
    union
    {
        UINT32 Flags;

        struct
        {
            UINT32 BlockingBySti : 1;
            UINT32 BlockingByMovSs : 1;
            UINT32 BlockingBySmi : 1;
            UINT32 BlockingByNmi : 1;
            UINT32 EnclaveInterruption : 1;
            UINT32 Reserved : 27;
        };
    };
} INTERRUPTIBILITY_STATE, *PINTERRUPTIBILITY_STATE;

/**
 * @brief Vm-exit interrupt info
 * 
 */
typedef union _VMEXIT_INTERRUPT_INFO
{
    struct
    {
        UINT32 Vector : 8;
        UINT32 InterruptionType : 3;
        UINT32 ErrorCodeValid : 1;
        UINT32 NmiUnblocking : 1;
        UINT32 Reserved : 18;
        UINT32 Valid : 1;
    };
    UINT32 Flags;
} VMEXIT_INTERRUPT_INFO, *PVMEXIT_INTERRUPT_INFO;

//////////////////////////////////////////////////
//				     Functions		      		//
//////////////////////////////////////////////////

VOID
IdtEmulationHandleExceptionAndNmi(UINT32 CurrentProcessorIndex, VMEXIT_INTERRUPT_INFO InterruptExit, PGUEST_REGS GuestRegs);

VOID
IdtEmulationHandleExternalInterrupt(UINT32 CurrentProcessorIndex, VMEXIT_INTERRUPT_INFO InterruptExit, PGUEST_REGS GuestRegs);

VOID
IdtEmulationHandleNmiWindowExiting(UINT32 CurrentProcessorIndex, PGUEST_REGS GuestRegs);

VOID
IdtEmulationHandleInterruptWindowExiting(UINT32 CurrentProcessorIndex);

BOOLEAN
IdtEmulationHandlePageFaults(UINT32                CurrentProcessorIndex,
                             VMEXIT_INTERRUPT_INFO InterruptExit,
                             UINT64                Address,
                             ULONG                 ErrorCode);
