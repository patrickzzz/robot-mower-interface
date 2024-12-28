## Possible operation modes

### Mainboard / CoverUI matrix of possible adapter connection variants

| Mainboard \ CoverUI                           | YF-Rev4-A\|B | YF-Rev6-A<br>via UART1[^1] | YF-Rev6-B<br>via UART2[^2] |
|-----------------------------------------------|:------------:|:--------------------------:|:--------------------------:|
| **OM UI Board**<br>via UART0[^3]              |       ✔      |           ✔               |          ✔                 |
| **YF-Rev4-A\|B**<br>No UART, read LED states  |       ✔      |           ✔               |          ✔                 |
| **YF-Rev6-A**<br>via UART1[^1]                |       ✔      |           ✘               |          ✔                 |
| **YF-Rev6-B**<br>via UART2[^2]                |       ✔      |           ✔               |          ✘                 |

[^1]: UART1 via TXD1 = GPIO17, RXD1 = GPIO18
[^2]: UART2 via TXDX = GPIO37, RXDX = GPIO38
[^3]: UART0 via TXD0 = GPIO43, RXD0 = GPIO44
