# Calculation

`Date of birth: 30`
## LM317 
VOUT1 = 2 + 0.01*30 = 2.3 V

### R_1 R_2
- **Datasheet Info**
`Equation: V_ref * (1+ R_1/R_2)+ I_adj * R_1`
`V_ref = 1.25 V`
`I_adj = 50 μA`

V_ref * (1+ R_1/R_2)+ I_adj * R_1 = 2.3 V
R_1 = 1.05 / (1.25/R_2 + 0.00005)

let R_2 = 240 Ω,
R_1 = 1.05 /(1.25/240+0.00005) ≈ 199.68 Ω ≈ 200 Ω  

- **Datasheet Info**
`V_F = 2 V`
`I_F = 2 mA`

### R_7
R_7 = (V_OUT1 - V_F) / I_F 

let I_F = 5 mA,
R_7 = 0.3 / 0.005 = 60 Ω 

---

## TPS79301 
VOUT2 = 0.1 + VOUT1 = 2.4 V

### R_3 R_4
- **Datasheet Info**
`V_OUT = V_REF * (1 + R_3/R_4)`
`V_REF = 1.2246 V`

R_3 = (2.4/1.2246 - 1) * R_4 = 0.9598 * R_4

let R_4 = 30.1 kΩ (recommended),
R_3 ≈ 28.89 kΩ

### R_8
- **Datasheet Info**
`V_F = 2 V`
`R_8 = (V_OUT1 - V_F) / I_F `

let I_F = 5 mA,
R_8 = 0.4 / 0.005 = 80 Ω 

---

## MIC5377 
VOUT3 = VOUT2 = 2.4 V

### R_5 R_6
- **Datasheet Info**
`V_OUT = V_REF * (1 + R_5/R_6)`
`V_REF = 1 V`

R_5 = (2.4 - 1) * R_6 = 1.4 * R_6

let R_6 = 100 kΩ,
R_6 = 140 kΩ

---

## Selected Resistors

| Resistors | Calculated | Selected |
|----|----|----|
| R1 | 200Ω | 200Ω |
| R2 | 240Ω | 240Ω |
| R3 | 28.89kΩ | 27.3kΩ |
| R4 | 30.1kΩ | 30.3kΩ |
| R5 | 100 kΩ | - |
| R6 | 140 kΩ | - |
| R7 | 60Ω  | 62.8Ω |
| R8 | 80Ω  | 82Ω |








