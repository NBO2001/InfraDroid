**Este é um guia estruturado para ajudá-lo a entender e implementar um protótipo de IR (Infravermelho) no Android Open Source Project (AOSP) utilizando a interface moderna AIDL.**



**Vamos dividir isso em camadas, da física até a compilação do sistema.**



**---**



**### 1. O Básico do Infravermelho: Física e Protocolo**



**Antes de escrever código, é crucial entender \_o que\_ estamos enviando. O IR não funciona apenas ligando e desligando uma luz como um código Morse simples. A luz ambiente (sol, lâmpadas) contém muito infravermelho, o que causaria interferência constante.**



**Para resolver isso, usamos \*\*Modulação\*\*.**



**- \*\*Frequência da Portadora (Carrier Frequency):\*\* O LED IR não fica apenas "aceso" para representar um bit. Ele pisca muito rápido em uma frequência específica (a mais comum é \*\*38kHz\*\*, mas varia entre 30kHz e 60kHz). O receptor IR é hardware-tuned para "enxergar" apenas essa frequência e ignorar a luz solar.**

    

**- \*\*O Padrão (Pattern):\*\* O "array de dados" que enviamos não são os bytes do arquivo (ex: 0xA1), mas sim uma sequência de \*\*tempos em microssegundos\*\*.**

    

    **- O protocolo define quanto tempo o LED deve emitir a frequência (Pulse/Mark) e quanto tempo deve ficar em silêncio (Space).**

        

    **- \_Exemplo:\_ `\[2000, 1000, 500, 500]` significa: Emitir 38kHz por 2000μs, silêncio por 1000μs, emitir por 500μs, silêncio por 500μs.**

        



**---**



**### 2. A Camada HAL (Hardware Abstraction Layer)**



**O Android não quer saber se você está usando um chip Qualcomm, MediaTek ou um FPGA customizado. Ele quer uma interface padrão. É aqui que entra a HAL.**



**A HAL define o contrato que o driver (seu hardware) deve cumprir para que o sistema (o Framework Android) possa usá-lo.**



**Localização da Interface:**



**A definição oficial fica em: hardware/interfaces/ir/aidl/aidl\_api/android.hardware.ir/current/android/hardware/ir/IConsumerIr.aidl**



**Esta interface AIDL expõe duas funções críticas que você deve implementar:**



**#### A. `android.hardware.ir.ConsumerIrFreqRange\[] getCarrierFreqs();`**



**- \*\*O que é:\*\* Esta função pergunta ao seu hardware: "Quais frequências de luz você consegue piscar?".**

    

**- \*\*Por que é importante:\*\* Nem todo LED IR consegue oscilar a 56kHz. O sistema precisa saber as limitações físicas do seu emissor para não tentar enviar um sinal que o hardware não suporta.**

    

**- \*\*Retorno:\*\* Um array de intervalos (ex: "Consigo de 30.000Hz até 60.000Hz").**

    



**#### B. `void transmit(in int carrierFreqHz, in int\[] pattern);`**



**- \*\*O que é:\*\* O comando de ação.**

    

**- \*\*Parâmetros:\*\***

    

    **- `carrierFreqHz`: A frequência da portadora (ex: 38000). O hardware deve configurar o PWM (Pulse Width Modulation) para esta frequência.**

        

    **- `pattern`: O array de inteiros alternando entre \_ON\_ e \_OFF\_ em microssegundos.**

        

**- \*\*Execução:\*\* O driver pega esse array e aciona o GPIO/PWM do hardware seguindo os tempos exatos definidos.**

    



**---**



**### 3. A Implementação de Referência (`default`)**



**O AOSP fornece uma implementação de exemplo chamada "default". Ela é essencialmente um \_Mock\_ ou \_Hello World\_. Ela compila e roda, mas não acende nenhum LED real; ela apenas simula o comportamento.**



**\*\*Localização:\*\* `hardware/interfaces/ir/aidl/default/`**



**Vamos analisar como essa implementação "fake" trata as funções. Geralmente, o código principal está em um arquivo como `ConsumerIr.cpp`.**



**#### Implementação de `getCarrierFreqs` no default:**



**No código de exemplo, a função simplesmente cria um vetor de dados sintéticos e o retorna. Ela diz ao sistema: "Eu sou um hardware falso que suporta de 30kHz a 60kHz".**



**C++**



**```**

**// Pseudocódigo explicativo da implementação default**

**ScopedAStatus ConsumerIr::getCarrierFreqs(std::vector<ConsumerIrFreqRange>\* \_aidl\_return) {**

    **// Cria um range fictício**

    **ConsumerIrFreqRange range;**

    **range.minHz = 30000; // 30kHz**

    **range.maxHz = 60000; // 60kHz**

    

    **// Retorna esse range para o sistema**

    **std::vector<ConsumerIrFreqRange> ranges = {range};**

    **\*\_aidl\_return = ranges;**

    **return ScopedAStatus::ok();**

**}**

**```**



**#### Implementação de `transmit` no default:**



**Como não há hardware real, a implementação default apenas recebe os dados e, geralmente, escreve no log (`logcat`) para provar que a chamada chegou até ali. Isso é perfeito para debug.**



**C++**



**```**

**// Pseudocódigo explicativo da implementação default**

**ScopedAStatus ConsumerIr::transmit(int32\_t carrierFreqHz, const std::vector<int32\_t>\& pattern) {**

    **// Verifica se a frequência é suportada (validar input)**

    **if (carrierFreqHz < 30000 || carrierFreqHz > 60000) {**

        **return ScopedAStatus::fromServiceSpecificError(ID\_ERROR); // Erro**

    **}**



    **// No hardware real, aqui você escreveria em /dev/ir\_rc ou sysfs.**

    **// No default, apenas logamos:**

    **LOG(INFO) << "Transmitindo IR na frequência: " << carrierFreqHz;**

    **LOG(INFO) << "Tamanho do padrão recebido: " << pattern.size();**



    **// Simula o tempo de envio (dorme pela duração do padrão)**

    **// ... logica de sleep ...**



    **return ScopedAStatus::ok();**

**}**

**```**



**\*\*Para o seu protótipo:\*\* Você copiará essa pasta `default`, renomeará para algo como `my\_device\_ir` e substituirá os logs pela lógica que escreve no arquivo do seu driver no Kernel (ex: `write()`).**



**---**



**### 4. Compilando o Módulo no seu Device**



**Para que o Android construa e inclua este binário na imagem do seu sistema, você precisa editar o arquivo `.mk` do seu produto (geralmente `device.mk` ou `product.mk` localizado em `device/<fabricante>/<modelo>/`).**



**O módulo de exemplo geralmente é definido no `Android.bp` dentro da pasta `default` com o nome `android.hardware.ir-service.example`.**



**\*\*Passo a passo:\*\***



**1. Abra o arquivo `.mk` principal do seu device (ex: `device.mk`).**

    

**2. Localize a variável `PRODUCT\_PACKAGES`.**

    

**3. Adicione o nome do serviço de exemplo à lista.**

    



**Makefile**



**```**

**# Dentro de device/<fabricante>/<modelo>/device.mk**



**PRODUCT\_PACKAGES += \\**

    **android.hardware.ir-service.example**

**```**



**PRODUCT\_COPY\_FILES += \\**

    **frameworks/native/data/etc/android.hardware.consumerir.xml:$(TARGET\_COPY\_OUT\_VENDOR)/etc/permissions/android.hardware.consumerir.xml**





**Isso garante que o binário executável seja compilado e colocado na partição `/vendor/bin/hw/`.**



**> \*\*Nota Importante:\*\* Para que o sistema inicie este serviço automaticamente no boot, o módulo também precisa de um arquivo de inicialização (`.rc`) e uma entrada no VINTF (Manifesto do Vendor). O módulo `android.hardware.ir-service.example` já costuma vir com um arquivo `ir-default.rc` e um `ir-default.xml` embutidos ou referenciados no `Android.bp` para facilitar isso.**

