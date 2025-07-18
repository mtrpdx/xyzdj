#Set your CCES version and root here
CCES_VERSION=3.0.2
DRIVE=c

#Set both options for legacy and new path versions
CCES_PATH_LEGACY_SHARC="${DRIVE}:\Analog Devices\CrossCore Embedded Studio ${CCES_VERSION}"
CCES_PATH_LEGACY_ARM="${DRIVE}:\Analog Devices\CrossCore Embedded Studio ${CCES_VERSION}\ARM\arm-none-eabi\bin"
CCES_PATH_SHARC="${DRIVE}:/analog/cces/${CCES_VERSION}"
CCES_PATH_ARM="${DRIVE}:/analog/cces/${CCES_VERSION}/ARM/arm-none-eabi/bin"

#Verify path
if [[ -d "$CCES_PATH_LEGACY_SHARC" && -d "$CCES_PATH_LEGACY_ARM" ]]; then
    export PATH="/${DRIVE}/Analog Devices/CrossCore Embedded Studio ${CCES_VERSION}":"/${DRIVE}/Analog Devices/CrossCore Embedded Studio ${CCES_VERSION}/ARM/arm-none-eabi/bin":$PATH
    #echo $PATH
    echo Using legacy CCES path: "Version: "$CCES_VERSION" on drive "$DRIVE":"
elif [[ -d "$CCES_PATH_SHARC" && -d "$CCES_PATH_ARM" ]]; then
    export PATH="/${DRIVE}/analog/cces/${CCES_VERSION}":"/${DRIVE}/analog/cces/${CCES_VERSION}/ARM/arm-none-eabi/bin":$PATH
    #echo $PATH
    echo Using new CCES path: "Version: "$CCES_VERSION" on drive "$DRIVE":"
else
    echo Could not locate CCES Version. Please double check path. 
fi
