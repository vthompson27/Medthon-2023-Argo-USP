# Medthon-2023
### Projeto de um protótipo de ventilador pulmonar de baixo custo para o Medthon 2023.

Desenvolvido em conjunto pela EP-USP, LEB-USP e IPT, o Medthon 2023 visava que os grupos participantes desenvolvessem um protótipo de ventilador pulmonar utilizando componentes de baixo custo e um microcontrolador ESP32. O principal desafio do Hackthon foi implementar um mecanismo de controle para estabelecer um patamar constante de pressão utilizando somente duas válvulas solenoides, uma reponsável pela entrada de ar no pulmão, inspiração, e a outra pela saída, expriação. Ao final do competição, eu e minha equipe, representando o grupo Argo de invoação em saúde, obtivemos a vitória

<div align="center">
<img src="https://github.com/vthompson27/Medthon-2023-Argo-USP/assets/79595013/efe911f3-04e1-4f84-b6da-78518aff2fe4" width="400px" />
</div>

<div align="center">
Imagem 1 - Foto do projeto ao lado do troféu.
</div>


_**Lista de componentes:**_
* **Pneumáticos:**
  - Mangueira PU, ⌀8mm, comprimentos:
    * 2x 4.5cm
    * 2x 6.0cm
    * 2x 8.0cm 
  - Mangueira PU, ⌀6mm, comprimentos:
    * 1x 4.0cm
    * 1x 3.0cm 
  * Mangueira PU, ⌀2mm
  * 2x Adaptador ⌀8mm<>⌀6mm
  * 1x Adaptador Y ⌀8mm
  * 1x Adaptador T ⌀8mm
  * 3x Adaptador Rosca 1⁄4 ⌀8mm (para bolsa térmica)
  * 1x Válvula reguladora de pressão 0-10 bar ⌀8mm
  * 1x Válvula solenoide DS2002 (Picanto 1.0 16v)
  * 1x Válvula feita em OpenSCAD (com atuador solenóide JF-0530B)
  * 1x Bolsa para Água quente Mercur (0.5L)
* **Eletrônicos**
    * 1x Microcontrolador ESP-WROOM-32 (ESP32 v1)
    * 2x Transistor NPN TIP1222
    * 1x Regulador de tensão L78053
    * 2x Diodos
    * 4x Resistores
    * 2x 1.5kOhm 
    * 1x 33kOhm 
    * 1x 18kOhm 
    * 2x Capacitores
    * 1x 1nF
    * 1x 100nF
    * 1x Sensor de pressão ABP001PDAAS Honeywell
    * 1x Kit breadboard com jumpers
* **Mecânicos**
    * 4x Abraçadeira plástica (descartável)
    * 4x Pés de borracha
    * 10x Parafuso (aprox M2) com porca
    * 20x Arroela (aprox M2)
    * 10x Arroela de travamento
    * 1x Conector para fonte de alimentação 12V (Terminal de parafuso)

 <div align="center">
<img src="https://github.com/vthompson27/Medthon-2023-Argo-USP/assets/79595013/e886fe74-bc68-423f-b219-a2fa501da019" width="400px" />
</div>

<div align="center">
Imagem 2 - Foto do circuito montado.
</div>

<br>

https://github.com/vthompson27/Medthon-2023-Argo-USP/assets/79595013/ed6f8412-740f-433e-941b-8aa5cb7eb120

<div align="center">
Vídeo 1 - Testes do projeto no Simulador pulmonar LS 2000.
</div>


  
