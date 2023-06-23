1、 内容
suguoxu@sgx:~$ cat /proc/interrupts
           CPU0       CPU1       CPU2       CPU3
  0:         10          0          0          0   IO-APIC   2-edge      timer
  1:        168        472          0       2981   IO-APIC   1-edge      i8042
  8:          1          0          0          0   IO-APIC   8-edge      rtc0
  9:         48         11          0          0   IO-APIC   9-fasteoi   acpi
 19:     283621    2678669      95994          0   IO-APIC  19-fasteoi   wlp8s0
 23:        180        106          0          0   IO-APIC  23-fasteoi   ehci_hcd:usb1
 24:          0          0          0          0   PCI-MSI 16384-edge      PCIe PME
 25:          0          0          0          0   PCI-MSI 458752-edge      PCIe PME
 26:          0          0          0          0   PCI-MSI 464896-edge      PCIe PME
 27:       1875      46586       2333         10   PCI-MSI 327680-edge      xhci_hcd
 28:        115         16          0          0   PCI-MSI 3670016-edge      rtsx_pci
 29:      28287      29579     268237          0   PCI-MSI 512000-edge      ahci[0000:00:1f.2]
 30:          0          0          0          0   PCI-MSI 3672064-edge      enp7s0f1
 31:       9216        962          0     110445   PCI-MSI 32768-edge      i915
 32:         28          0          0          0   PCI-MSI 524288-edge      nvkm
 33:          0          0          0          0   PCI-MSI 49152-edge      snd_hda_intel:card0
 34:          0          0         11          0   PCI-MSI 360448-edge      mei_me
 35:          0          0          0        189   PCI-MSI 442368-edge      snd_hda_intel:card1
NMI:         11        141        183        129   Non-maskable interrupts
LOC:   13303103    5769918   14250312    7541979   Local timer interrupts
SPU:          0          0          0          0   Spurious interrupts
PMI:         11        141        183        129   Performance monitoring interrupts
IWI:       2776        345        283      34550   IRQ work interrupts
RTR:          0          0          0          0   APIC ICR read retries
RES:     185980     150451     157868      68422   Rescheduling interrupts
CAL:     139872      66271      84990      75710   Function call interrupts
TLB:      71627      61594      76759      67043   TLB shootdowns
TRM:         65         65         65         65   Thermal event interrupts
THR:          0          0          0          0   Threshold APIC interrupts
DFR:          0          0          0          0   Deferred Error APIC interrupts
MCE:          0          0          0          0   Machine check exceptions
MCP:        473        470        470        470   Machine check polls
ERR:          0
MIS:          0
PIN:          0          0          0          0   Posted-interrupt notification event
NPI:          0          0          0          0   Nested posted-interrupt event
PIW:          0          0          0          0   Posted-interrupt wakeup event
suguoxu@sgx:~$

    /proc/interrupts中的字段依次是逻辑中断号、中断在各CPU上发生的次数，中断所属父设备名称、硬件中断号、中断触发方式(电平或边沿)、中断名称