// SPDX-License-Identifier: MIT

pragma solidity ^0.8.3;
/*
contract IpfsStorage {
 mapping (address => string) public userFiles;
 function setFileIPFS(string memory file) external {
 userFiles[msg.sender] = file;
 }
}*/

contract SensorAlert {

    // Mapeo para almacenar el hash de los archivos y sus correspondientes alertas
    mapping(address => string) public userFiles;
    mapping(address => bool) public alertStatus;

    // Evento que se emite cuando una alerta es activada
    event SensorAlertTriggered(address indexed user, string fileHash, uint256 timestamp, string sensorType, uint256 sensorValue);

    // Función para almacenar el hash del archivo (se usa en Node-RED para almacenar el hash)
    function setFileIPFS(string memory fileHash) external {
        userFiles[msg.sender] = fileHash;
    }

    // Función para comprobar los valores de los sensores
    function checkSensorsAndTriggerAlert(
        string memory fileHash,
        uint256 pirValue,
        uint256 vibration1Value,
        uint256 vibration2Value,
        uint256 magneticoValue
    ) external {
        // Comprobamos los valores de los sensores
        if (pirValue == 1) {
            emit SensorAlertTriggered(msg.sender, fileHash, block.timestamp, "PIR", pirValue);
            alertStatus[msg.sender]==true;
        }
        if (vibration1Value == 1) {
            emit SensorAlertTriggered(msg.sender, fileHash, block.timestamp, "Vibration1", vibration1Value);
            alertStatus[msg.sender]==true;
        }
        if (vibration2Value == 1) {
            emit SensorAlertTriggered(msg.sender, fileHash, block.timestamp, "Vibration2", vibration2Value);
            alertStatus[msg.sender]==true;
        }
        if (magneticoValue == 1) {
            emit SensorAlertTriggered(msg.sender, fileHash, block.timestamp, "Magnetico", magneticoValue);
            alertStatus[msg.sender]==true;
        }
    }

    // Función para obtener el hash del archivo subido
    function getFileHash() external view returns (string memory) {
        return userFiles[msg.sender];
    }

    // Función para obtener el estado de la alerta de un usuario
    function getAlertStatus() external view returns (bool) {
        return alertStatus[msg.sender];
    }
}
