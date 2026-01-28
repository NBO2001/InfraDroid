# Herda as configurações do emulador (produto sdk_phone_x86_64)
$(call inherit-product, $(SRC_TARGET_DIR)/product/sdk_phone_x86_64.mk)



# Sobrescreve algumas variáveis com os dados do novo produto
PRODUCT_NAME := devtitans_kraken
PRODUCT_DEVICE := kraken
PRODUCT_BRAND := KrakenBrand
PRODUCT_MODEL := KrakenModel



# Copia o arquivo devtitans.txt para o /system/etc da imagem do Android
PRODUCT_COPY_FILES += \
    device/devtitans/kraken/devtitans.txt:system/etc/devtitans.txt \
    device/devtitans/kraken/kraken.rc:vendor/etc/init/kraken.rc \
    device/devtitans/kraken/bootanimation.zip:product/media/bootanimation.zip

PRODUCT_PACKAGES += \
    UniversalMediaPlayer \
    hello_c \
    nano \
    sl \
    hello_cpp \
    hello_daemon_cpp \
    HelloApp \
    HelloJava \
    smartlamp_client \
    devtitans.smartlampmanager \

    
BOARD_SEPOLICY_DIRS += device/devtitans/kraken/sepolicy


PRODUCT_COPY_FILES += \
    device/devtitans/kraken/default_wallpaper.png:product/media/default_wallpaper.png

PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.wallpaper=/product/media/default_wallpaper.png

PRODUCT_SYSTEM_PROPERTIES += \
    ro.devtitans.name=Kraken

PRODUCT_PRODUCT_PROPERTIES += \
    ro.product.devtitans.version=1.0

PRODUCT_VENDOR_PROPERTIES += \
    ro.vendor.devtitans.hardware=ModelB


PRODUCT_PACKAGE_OVERLAYS = device/devtitans/kraken/overlay


# Smartlamp AIDL Interface
PRODUCT_PACKAGES += devtitans.smartlamp





# Smartlamp Binder Service
PRODUCT_PACKAGES += devtitans.smartlamp-service

# Device Framework Matrix (Declara que o nosso produto Kraken precisa do serviço smartlamp)
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE := device/devtitans/kraken/device_framework_matrix.xml



# Cliente de Linha de Comando para o Serviço Smartlamp
PRODUCT_PACKAGES += smartlamp_service_client

# App Privilegiado de Teste do Serviço Smartlamp
PRODUCT_PACKAGES += SmartlampTestApp






#aidl interface
PRODUCT_PACKAGES += devtitans.infradroid

#binder
PRODUCT_PACKAGES += devtitans.consumerir

