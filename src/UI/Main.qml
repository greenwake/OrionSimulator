import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: mainWindow
    width: 1920
    height: 1080
    visible: true
    title: qsTr("Orion Shooting Simulator")
    color: "#000000" // Simülatör için siyah arka plan
    visibility: Window.FullScreen // Tam ekran modu

    // C++'tan gelen verileri görselleştirmek için property'ler
    property int currentScore: 0
    property string currentMode: "Bekleniyor..."
    property point lastHitPoint: Qt.point(0, 0)

    // Arka Plan (Grid - Poligon havası katmak için)
    Item {
        anchors.fill: parent
        opacity: 0.3
        Repeater {
            model: 20
            delegate: Rectangle {
                x: index * (parent.width / 20)
                width: 1
                height: parent.height
                color: "#00FF00"
            }
        }
        Repeater {
            model: 12
            delegate: Rectangle {
                y: index * (parent.height / 12)
                width: parent.width
                height: 1
                color: "#00FF00"
            }
        }
    }

    // --- C++ SİNYAL BAĞLANTILARI ---
    Connections {
        // main.cpp içinde setContextProperty("sceneManager", ...) demiştik.
        target: sceneManager

        // C++: emit scoreChanged(int newScore)
        function onScoreChanged(newScore) {
            currentScore = newScore
        }

        // C++: emit targetHit(float x, float y, bool isHit)
        function onTargetHit(x, y, isHit) {
            // Gelen 0.0-1.0 verisini ekran çözünürlüğüne çevir
            var screenX = x * mainWindow.width
            var screenY = y * mainWindow.height
            console.log(x);
            console.log(y);
            console.log(screenX);
            console.log(screenY);

            lastHitPoint = Qt.point(screenX, screenY)

            // Vuruş görseli oluştur (Hit Marker)
            hitMarkerAnim.restart()

            if (isHit) {
                console.log("Hedef Vuruldu!")
                // Burada bir ses efekti çalınabilir
            }
        }
    }

    // --- HUD (HEADS UP DISPLAY) ---
    RowLayout {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        spacing: 50
        z: 100 // En üstte görünsün

        // Skor Tablosu
        Rectangle {
            width: 300; height: 80
            color: "#202020"
            border.color: "#00FF00"
            border.width: 2
            radius: 10

            Text {
                anchors.centerIn: parent
                text: "SKOR: " + currentScore
                color: "#00FF00"
                font.pixelSize: 40
                font.bold: true
                font.family: "Courier New"
            }
        }

        // Mod Göstergesi
        Rectangle {
            width: 400; height: 80
            color: "#202020"
            border.color: "orange"
            border.width: 2
            radius: 10

            Text {
                anchors.centerIn: parent
                text: "MOD: " + currentMode
                color: "orange"
                font.pixelSize: 30
                font.bold: true
                font.family: "Courier New"
            }
        }
    }

    // --- SAHNE ALANI (Burada hedefler görünecek) ---
    Item {
        id: gameArea
        anchors.fill: parent

        // ÖRNEK HEDEF (Test için statik bir daire)
        Rectangle {
            id: testTarget
            width: 150; height: 150
            radius: 75
            color: "red"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2

            // Hedefin üzerinde yazı
            Text {
                anchors.centerIn: parent
                text: "HEDEF"
                color: "white"
                font.bold: true
            }

            // Basit bir animasyon (Nefes alma efekti)
            SequentialAnimation {
                running: true
                loops: Animation.Infinite
                NumberAnimation { target: testTarget; property: "scale"; to: 1.2; duration: 1000 }
                NumberAnimation { target: testTarget; property: "scale"; to: 1.0; duration: 1000 }
            }
        }
    }

    // --- VURUŞ GÖSTERGESİ (HIT MARKER) ---
    // Atış yapıldığında ekranda beliren kırmızı nokta
    Rectangle {
        id: hitMarker
        z:999
        width: 20; height: 20
        radius: 10
        color: "yellow"
        x: lastHitPoint.x - (width / 2)
        y: lastHitPoint.y - (height / 2)
        visible: false // Sadece atış gelince görünür

        // Vuruş efekti animasyonu
        ParallelAnimation {
            id: hitMarkerAnim
            ScriptAction { script: hitMarker.visible = true }
            NumberAnimation { target: hitMarker; property: "opacity"; from: 1.0; to: 0.0; duration: 500 }
            NumberAnimation { target: hitMarker; property: "scale"; from: 1.0; to: 3.0; duration: 500 }
            ScriptAction { script: hitMarker.visible = true }
        }
    }

    // --- MOUSE İLE TEST (DEBUG) ---
    // Sensör yokken mouse ile tıklayarak sistemi test etmek için
    MouseArea {
        anchors.fill: parent
        onClicked: (mouse) => {
            // Mouse koordinatlarını 0-1 arasına normalize et
            var normX = mouse.x / width
            var normY = mouse.y / height

            // C++ tarafına sinyal gönderiyormuş gibi simüle et
            // Normalde bu sceneManager.handleInput() olurdu

            // Görsel test için:
            lastHitPoint = Qt.point(mouse.x, mouse.y)
            hitMarkerAnim.restart()
            currentScore += 10 // Skoru artır (Test)
        }
    }
}
