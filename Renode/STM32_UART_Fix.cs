//
// Copyright (c) 2010-2023 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//
using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;
using System.Collections.Generic;
using Antmicro.Migrant;
using Antmicro.Migrant.Hooks;
using Antmicro.Renode.Core.Structure.Registers;

namespace Antmicro.Renode.Peripherals.UART
{
    public class STM32_UART_Fix : UARTBase, IUARTWithBufferState, IDoubleWordPeripheral, IWordPeripheral, IBytePeripheral
    {
        public STM32_UART_Fix(Machine machine, uint frequency = 8000000) : base(machine)
        {
            IRQ = new GPIO();
            ReceiveDmaRequest = new GPIO();
            registers = new DoubleWordRegisterCollection(this, BuildRegisterMap());
            this.frequency = frequency;
        }

        public override void Reset()
        {
            base.Reset();
            registers.Reset();
            IRQ.Unset();
            ReceiveDmaRequest.Unset();
        }

        public uint ReadDoubleWord(long offset)
        {
            return registers.Read(offset);
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            registers.Write(offset, value);
        }

        public ushort ReadWord(long offset)
        {
            return (ushort)registers.Read(offset);
        }

        public void WriteWord(long offset, ushort value)
        {
            registers.Write(offset, value);
        }

        public byte ReadByte(long offset)
        {
            return (byte)registers.Read(offset);
        }

        public void WriteByte(long offset, byte value)
        {
            registers.Write(offset, value);
        }

        public override uint BaudRate
        {
            get
            {
                //OversamplingMode.By8 means we ignore the oldest bit of dividerFraction.Value
                var fraction = oversamplingMode.Value == OversamplingMode.By16 ? dividerFraction.Value : dividerFraction.Value & 0b111;

                var divisor = 8 * (2 - (int)oversamplingMode.Value) * (dividerMantissa.Value + fraction / 16.0);
                return divisor == 0 ? 0 : (uint)(frequency / divisor);
            }
        }

        public override Bits StopBits
        {
            get
            {
                switch(stopBits.Value)
                {
                case StopBitsValues.Half:
                    return Bits.Half;
                case StopBitsValues.One:
                    return Bits.One;
                case StopBitsValues.OneAndAHalf:
                    return Bits.OneAndAHalf;
                case StopBitsValues.Two:
                    return Bits.Two;
                default:
                    throw new ArgumentException("Invalid stop bits value");
                }
            }
        }

        public override Parity ParityBit => parityControlEnabled.Value ?
                                    (paritySelection.Value == ParitySelection.Even ?
                                        Parity.Even :
                                        Parity.Odd) :
                                    Parity.None;

        public BufferState BufferState
        {
            get
            {
                return bufferState;
            }

            private set
            {
                if(bufferState == value)
                {
                    return;
                }
                bufferState = value;
                UpdateInterrupt();
                rdrNotEmpty.Value = (value != BufferState.Empty);
                BufferStateChanged?.Invoke(value);
                ReceiveDmaRequest.Set(receiveDmaEnabled.Value && rdrNotEmpty.Value);
            }
        }

        public event Action<BufferState> BufferStateChanged;

        public GPIO IRQ { get; } = new GPIO();
        public GPIO ReceiveDmaRequest { get; }

        public DoubleWordRegisterCollection registers{ get; }
        
        protected override void CharWritten()
        {
            BufferState = BufferState.Ready;
        }

        protected override void QueueEmptied()
        {
            BufferState = BufferState.Empty;
        }

        protected override bool IsReceiveEnabled => receiverEnabled.Value && usartEnabled.Value;

        private Dictionary<long, DoubleWordRegister> BuildRegisterMap()
        {
            var registersMap = new Dictionary<long, DoubleWordRegister>
            {
                {(long)Registers.Status, new DoubleWordRegister(this, resetValue: 0xC0)
                    .WithTaggedFlag("PE", 0)
                    .WithTaggedFlag("FE", 1)
                    .WithTaggedFlag("NF", 2)
                    .WithFlag(3, FieldMode.Read, valueProviderCallback: _ => false, name: "ORE") // we assume no receive overruns
                    .WithTaggedFlag("IDLE", 4)
                    .WithFlag(5, out rdrNotEmpty, FieldMode.Read, valueProviderCallback: _ => (Count != 0), name: "RXNE") // as these two flags are WZTC, we cannot just calculate their results
                    .WithFlag(6, out transmissionComplete, FieldMode.Read, name: "TC")
                    .WithFlag(7, FieldMode.Read, valueProviderCallback: _ => true, name: "TXE") // we always assume "transmit data register empty"
                    .WithTaggedFlag("LBD", 8)
                    .WithTaggedFlag("CTS", 9)
                    .WithReservedBits(10, 22)
                },
                {(long)Registers.Data, new DoubleWordRegister(this)
                    .WithValueField(0, 9, 
                        valueProviderCallback: _ => HandleReceiveData(), 
                        writeCallback: (_, value) => HandleTransmitData((uint)value),
                        name: "DR"
                    )
                },
                {(long)Registers.BaudRate, new DoubleWordRegister(this)
                    .WithValueField(0, 4, out dividerFraction, name: "DIV_Fraction")
                    .WithValueField(4, 12, out dividerMantissa, name: "DIV_Mantissa")
                },
                {(long)Registers.Control1, new DoubleWordRegister(this)
                    .WithTaggedFlag("SBK", 0)
                    .WithTaggedFlag("RWU", 1)
                    .WithFlag(2, out receiverEnabled, name: "RE")
                    .WithFlag(3, out transmitterEnabled, name: "TE")
                    .WithTaggedFlag("IDLEIE", 4)
                    .WithFlag(5, out receiverNotEmptyInterruptEnabled, name: "RXNEIE")
                    .WithFlag(6, out transmissionCompleteInterruptEnabled, name: "TCIE")
                    .WithFlag(7, out transmitDataRegisterEmptyInterruptEnabled, name: "TXEIE")
                    .WithTaggedFlag("PEIE", 8)
                    .WithEnumField(9, 1, out paritySelection, name: "PS")
                    .WithFlag(10, out parityControlEnabled, name: "PCE")
                    .WithTaggedFlag("WAKE", 11)
                    .WithTaggedFlag("M", 12)
                    .WithFlag(13, out usartEnabled, name: "UE")
                    .WithReservedBits(14, 1)
                    .WithEnumField(15, 1, out oversamplingMode, name: "OVER8")
                    .WithReservedBits(16, 16)
                    .WithWriteCallback((_, __) => UpdateInterrupt())
                },
                {(long)Registers.Control2, new DoubleWordRegister(this)
                    .WithTag("ADD", 0, 4)
                    .WithReservedBits(5, 1)
                    .WithTaggedFlag("LBDIE", 6)
                    .WithReservedBits(7, 1)
                    .WithTaggedFlag("LBCL", 8)
                    .WithTaggedFlag("CPHA", 9)
                    .WithTaggedFlag("CPOL", 10)
                    .WithTaggedFlag("CLKEN", 11)
                    .WithEnumField(12, 2, out stopBits, name: "STOP")
                    .WithTaggedFlag("LINEN", 14)
                    .WithReservedBits(15, 17)
                },
                {(long)Registers.Control3, new DoubleWordRegister(this)
                    .WithTaggedFlag("EIE", 0)
                    .WithTaggedFlag("IREN", 1)
                    .WithTaggedFlag("IRLP", 2)
                    .WithTaggedFlag("HDSEL", 3)
                    .WithTaggedFlag("NACK", 4)
                    .WithTaggedFlag("SCEN", 5)
                    .WithTaggedFlag("DMAR", 6)
                    .WithFlag(7, out receiveDmaEnabled, name: "DMAT")
                    .WithTaggedFlag("RTSE", 8)
                    .WithTaggedFlag("CTSE", 9)
                    .WithTaggedFlag("CTSIE", 10)
                    .WithTaggedFlag("ONEBIT", 11)
                    .WithReservedBits(12, 20)
                }
            };

            return registersMap;
        }

        private void HandleTransmitData(uint value)
        {
            if(transmitterEnabled.Value && usartEnabled.Value)
            {
                base.TransmitCharacter((byte)value);
                transmissionComplete.Value = true;
                UpdateInterrupt();
            }
            else
            {
                this.Log(LogLevel.Warning, "Char was to be sent, but the transmitter (or the whole USART) is not enabled. Ignoring.");
            }
        }

        private uint HandleReceiveData()
        {
            if(!TryGetCharacter(out var result))
            {
                this.Log(LogLevel.Warning, "No characters in queue.");
            }
            return result;
        }

        private void UpdateInterrupt()
        {
            var transmitRegisterEmptyInterrupt = transmitDataRegisterEmptyInterruptEnabled.Value; // we assume that transmit register is always empty
            var transmissionCompleteInterrupt = transmissionComplete.Value && transmissionCompleteInterruptEnabled.Value;
            var receiverNotEmptyInterrupt = Count != 0 && receiverNotEmptyInterruptEnabled.Value;
            
            IRQ.Set(transmitRegisterEmptyInterrupt || transmissionCompleteInterrupt || receiverNotEmptyInterrupt);
        }
    
        private IEnumRegisterField<OversamplingMode> oversamplingMode;
        private IEnumRegisterField<StopBitsValues> stopBits;
        private IFlagRegisterField usartEnabled;
        private IFlagRegisterField parityControlEnabled;
        private IEnumRegisterField<ParitySelection> paritySelection;
        private IFlagRegisterField transmissionCompleteInterruptEnabled;
        private IFlagRegisterField transmitDataRegisterEmptyInterruptEnabled;
        private IFlagRegisterField receiverNotEmptyInterruptEnabled;
        private IFlagRegisterField receiverEnabled;
        private IFlagRegisterField transmitterEnabled;
        private IFlagRegisterField rdrNotEmpty;
        private IFlagRegisterField receiveDmaEnabled;
        private IFlagRegisterField transmissionComplete;
        private IValueRegisterField dividerMantissa;
        private IValueRegisterField dividerFraction;

        private BufferState bufferState;
        private readonly uint frequency;

        private enum OversamplingMode
        {
            By16 = 0,
            By8 = 1
        }

        private enum StopBitsValues
        {
            One = 0,
            Half = 1,
            Two = 2,
            OneAndAHalf = 3
        }

        private enum ParitySelection
        {
            Even = 0,
            Odd = 1
        }

        private enum Registers : long
        {
            Status = 0x00,
            Data = 0x04,
            BaudRate = 0x08,
            Control1 = 0x0C,
            Control2 = 0x10,
            Control3 = 0x14,
            GuardTimeAndPrescaler = 0x18
        }
    }
}
