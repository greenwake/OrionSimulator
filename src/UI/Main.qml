import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Particles
import QtMultimedia

Window {
    id: mainWindow
    width: 1920
    height: 1080
    visible: true
    title: qsTr("Orion Shooting Simulator")
    color: "#000000"
    visibility: Window.FullScreen

    // --- DURUM DEĞİŞKENLERİ ---
    property bool isGameRunning: false
    property int selectedPlayerCount: 1
    property string selectedMode: "Refleks"
    property bool isGameOver: false
    property var gameReport: null
    property int currentScoreP1: 0
    property int currentScoreP2: 0

    property string player1Name: p1NameInput.text === "" ? "1. OYUNCU" : p1NameInput.text.toUpperCase()
    property string player2Name: p2NameInput.text === "" ? "2. OYUNCU" : p2NameInput.text.toUpperCase()

    property int currentLevel: 1
    property string announcementText: ""

    property point lastHitPoint: Qt.point(0, 0)

    // C++ SİNYAL BAĞLANTILARI
    Connections {
        target: sceneManager
        function onTargetHit(x, y, isHit) {
            if (!isGameRunning) return;

            var screenX = x * mainWindow.width
            var screenY = y * mainWindow.height

            bulletHoleModel.append({
                "xPos": screenX,
                "yPos": screenY
            });

            lastHitPoint = Qt.point(screenX, screenY)
            hitMarkerAnim.restart()
        }
        function onScoreChanged(playerId, newScore) {
            if (playerId === 1) currentScoreP1 = newScore;
            else if (playerId === 2) currentScoreP2 = newScore;
        }

        function onTargetSpawned(id, x, y, size, color) {
            if (!isGameRunning) return;
            targetModel.append({
                "targetId": id,
                "xPos": (x * mainWindow.width) - (size / 2),
                "yPos": (y * mainWindow.height) - (size / 2),
                "size": size,
                "targetColor": color
            });
        }

        // C++'tan süre bitti veya vuruldu bilgisi gelince listeden sil
        function onTargetRemoved(id, wasHit) { // YENİ: wasHit eklendi
            var targetX = 0;
            var targetY = 0;
            var targetColor = "white";
            var found = false;

            for (var i = 0; i < targetModel.count; i++) {
                var item = targetModel.get(i);
                if (item.targetId === id) {
                    targetX = item.xPos + (item.size / 2);
                    targetY = item.yPos + (item.size / 2);
                    targetColor = item.targetColor;

                    targetModel.remove(i);
                    found = true;
                    break;
                }
            }

            // Eğer hedef listede bulunduysa
            if (found) {
                // YENİ: Sadece eğer VURULDUYSA patlat! (Süresi dolanlar sessizce silinir)
                if (wasHit) {
                    explosionSystem.sparkColor = targetColor;
                    explosionEmitter.x = targetX;
                    explosionEmitter.y = targetY;
                    explosionEmitter.burst(50); // PATLAT!
                }
            }
        }

        function onLevelChanged(newLevel) {
            currentLevel = newLevel;
            bulletHoleModel.clear();
        }

        function onRoundWinner(playerId, message) {
        // Hangi oyuncu kazandıysa onun adını al
        var wName = (playerId === 1) ? player1Name : player2Name;

        // C++'tan gelen mesajda "BİTTİ" kelimesi varsa oyun komple bitmiştir
        if (message.indexOf("BİTTİ") !== -1) {
            announcementText = "OYUN BİTTİ!\nKAZANAN: " + wName;
        } else {
            announcementText = wName + "\n" + currentLevel + ". SEVİYEYİ KAZANDI!";
        }
        announcementAnim.restart();
    }
        function onGameOver(report) {
            gameReport = report;
            isGameOver = true; // Rapor ekranını açar
        }

        // TCP'den { "player1name": "user1" ... } mesajı geldiğinde çalışır
        function onPlayerConfigUpdated(playerCount, p1Name, p2Name) {
            // Butonlardaki seçimi otomatik değiştirir (1 Oyuncu / 2 Oyuncu)
            selectedPlayerCount = playerCount;

            // 1. Oyuncunun ismini metin kutusuna yazar
            p1NameInput.text = p1Name;

            if (playerCount === 2) {
                // Eğer 2. oyuncu da JSON'da gönderildiyse onu da yazar
                p2NameInput.text = p2Name;
            } else {
                // Eğer tek oyuncu gönderildiyse 2. kutuyu temizler
                p2NameInput.text = "2. OYUNCU";
            }

            // DİKKAT: JSON mesajı geldiğinde menüde beklemeden otomatik oyunu
            // başlatmak isterseniz aşağıdaki iki satırın başındaki yorum satırını kaldırabilirsiniz:
            // sceneManager.startGame(selectedPlayerCount, selectedMode)
            // isGameRunning = true
        }
    }

    Item {
        anchors.fill: parent
        focus: true
        Keys.onEscapePressed: {
            isGameRunning = false
        }
    }

    // =========================================================
    // 1. ANA MENÜ EKRANI
    // =========================================================
    Item {
        id: menuScreen
        anchors.fill: parent
        visible: !isGameRunning

        Rectangle { anchors.fill: parent; color: "#050505" }

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 40

            Text {
                text: "ORION ATIŞ SİMÜLATÖRÜ"
                color: "#00FF00"
                font.pixelSize: 60
                font.bold: true
                font.family: "Courier New"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 30
            }

            Text { text: "OYUNCU SAYISI"; color: "white"; font.pixelSize: 24; Layout.alignment: Qt.AlignHCenter }
            RowLayout {
                spacing: 20
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    width: 200; height: 60
                    color: selectedPlayerCount === 1 ? "#00FF00" : "#202020"
                    border.color: "#00FF00"; border.width: 2; radius: 5
                    Text { anchors.centerIn: parent; text: "1 OYUNCU"; color: selectedPlayerCount === 1 ? "black" : "#00FF00"; font.pixelSize: 20; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: selectedPlayerCount = 1 }
                }
                Rectangle {
                    width: 200; height: 60
                    color: selectedPlayerCount === 2 ? "#00FF00" : "#202020"
                    border.color: "#00FF00"; border.width: 2; radius: 5
                    Text { anchors.centerIn: parent; text: "2 OYUNCU"; color: selectedPlayerCount === 2 ? "black" : "#00FF00"; font.pixelSize: 20; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: selectedPlayerCount = 2 }
                }
            }

            Text { text: "EĞİTİM MODU"; color: "white"; font.pixelSize: 24; Layout.alignment: Qt.AlignHCenter; Layout.topMargin: 20 }
            RowLayout {
                spacing: 20
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    width: 200; height: 60
                    color: selectedMode === "Refleks" ? "orange" : "#202020"
                    border.color: "orange"; border.width: 2; radius: 5
                    Text { anchors.centerIn: parent; text: "REFLEKS"; color: selectedMode === "Refleks" ? "black" : "orange"; font.pixelSize: 20; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: selectedMode = "Refleks" }
                }
                Rectangle {
                    width: 200; height: 60
                    color: selectedMode === "Poligon" ? "orange" : "#202020"
                    border.color: "orange"; border.width: 2; radius: 5
                    Text { anchors.centerIn: parent; text: "POLİGON"; color: selectedMode === "Poligon" ? "black" : "orange"; font.pixelSize: 20; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: selectedMode = "Poligon" }
                }
                Rectangle {
                    width: 200; height: 60
                    color: selectedMode === "Matematik" ? "orange" : "#202020"
                    border.color: "orange"; border.width: 2; radius: 5
                    Text { anchors.centerIn: parent; text: "MATEMATİK"; color: selectedMode === "Matematik" ? "black" : "orange"; font.pixelSize: 20; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: selectedMode = "Matematik" }
                }
            }
            // --- OYUNCU İSİMLERİ (YENİ EKLENDİ) ---
            Text { text: "OYUNCU İSİMLERİ"; color: "white"; font.pixelSize: 24; Layout.alignment: Qt.AlignHCenter; Layout.topMargin: 20 }
            RowLayout {
                spacing: 20
                Layout.alignment: Qt.AlignHCenter

                // 1. Oyuncu İsim Kutusu
                TextField {
                    id: p1NameInput
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 60
                    placeholderText: "1. Oyuncu Adı"
                    text: "1. OYUNCU"
                    color: "#00FF00"
                    font.pixelSize: 20
                    font.bold: true
                    horizontalAlignment: TextInput.AlignHCenter
                    background: Rectangle { color: "#202020"; border.color: "#00FF00"; border.width: 2; radius: 5 }
                }

                // 2. Oyuncu İsim Kutusu (Sadece 2. oyuncu seçiliyse görünür)
                TextField {
                    id: p2NameInput
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 60
                    placeholderText: "2. Oyuncu Adı"
                    text: "2. OYUNCU"
                    color: "cyan"
                    font.pixelSize: 20
                    font.bold: true
                    horizontalAlignment: TextInput.AlignHCenter
                    visible: selectedPlayerCount === 2
                    background: Rectangle { color: "#202020"; border.color: "cyan"; border.width: 2; radius: 5 }
                }
            }

            Rectangle {
                width: 300; height: 80
                color: "#FF0000"
                radius: 10
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 50
                Text { anchors.centerIn: parent; text: "SİSTEMİ BAŞLAT"; color: "white"; font.pixelSize: 28; font.bold: true }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        sceneManager.startGame(selectedPlayerCount, selectedMode)
                        isGameRunning = true
                    }
                }
            }

            Text {
                text: "Çıkmak için klavyeden 'ESC'ye basın."
                color: "gray"
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 20
            }
        }
    }

    // =========================================================
    // 2. OYUN EKRANI
    // =========================================================
    Item {
        id: gameScreen
        anchors.fill: parent
        visible: isGameRunning

        Item {
            anchors.fill: parent
            opacity: 0.3
            Repeater { model: 20; delegate: Rectangle { x: index * (parent.width / 20); width: 1; height: parent.height; color: "#00FF00" } }
            Repeater { model: 12; delegate: Rectangle { y: index * (parent.height / 12); width: parent.width; height: 1; color: "#00FF00" } }
        }

        Rectangle {
            id: middleDivider
            width: 8
            height: parent.height
            color: "cyan"
            x: (parent.width - width) / 2
            visible: selectedPlayerCount === 2
            z: 10
            Rectangle {
                width: 20
                height: parent.height
                color: "cyan"
                opacity: 0.3
                anchors.centerIn: parent
            }
        }

        Text {
            text: player1Name
            color: "#00FF00"
            opacity: 0.1
            font.pixelSize: 120
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            x: selectedPlayerCount === 2 ? (parent.width / 4) - (width / 2) : (parent.width / 2) - (width / 2)
            z: 5
        }

        Text {
            text: player2Name
            color: "cyan"
            opacity: 0.1
            font.pixelSize: 120
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            x: (parent.width * 0.75) - (width / 2)
            visible: selectedPlayerCount === 2
            z: 5
        }


        RowLayout {
            anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 20
            z: 100

            Rectangle {
                width: 300; height: 80; color: "#202020"; border.color: "#00FF00"; border.width: 2; radius: 10
                Layout.alignment: Qt.AlignLeft
                Text { anchors.centerIn: parent; text: player1Name + ": " + currentScoreP1; color: "#00FF00"; font.pixelSize: 30; font.bold: true }
            }

            // SEVİYE GÖSTERGESİ (SOLA YASLI)
            Rectangle {
                width: 200; height: 80; color: "#202020"; border.color: "yellow"; border.width: 2; radius: 10
                Layout.alignment: Qt.AlignLeft
                Text { anchors.centerIn: parent; text: "LVL: " + currentLevel; color: "yellow"; font.pixelSize: 30; font.bold: true }
            }

            Rectangle {
                width: 400; height: 80; color: "#202020"; border.color: "orange"; border.width: 2; radius: 10
                Layout.alignment: Qt.AlignHCenter
                Text { anchors.centerIn: parent; text: "MOD: " + selectedMode.toUpperCase(); color: "orange"; font.pixelSize: 30; font.bold: true }
            }

            Rectangle {
                width: 300; height: 80; color: "#202020"; border.color: "cyan"; border.width: 2; radius: 10
                Layout.alignment: Qt.AlignRight
                visible: selectedPlayerCount === 2
                Text { anchors.centerIn: parent; text: player2Name + ": " + currentScoreP2; color: "cyan"; font.pixelSize: 30; font.bold: true }
            }
        }

        ListModel { id: targetModel }

        Repeater {
            model: targetModel
            delegate: Rectangle {
                property int targetId: model.targetId // EKSİK OLAN KISIM EKLENDİ

                width: model.size; height: model.size; radius: model.size / 2
                x: model.xPos; y: model.yPos
                color: model.targetColor

                scale: 0.0
                Component.onCompleted: popAnim.start()

                NumberAnimation on scale {
                    id: popAnim
                    from: 0.0; to: 1.0; duration: 300; easing.type: Easing.OutBack
                }

                SequentialAnimation on scale {
                    running: !popAnim.running
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.1; duration: 800 }
                    NumberAnimation { to: 1.0; duration: 800 }
                }
            }
        }
        // --- KALICI MERMİ İZLERİ (BULLET HOLES) ---
        ListModel { id: bulletHoleModel }

        Repeater {
            model: bulletHoleModel
            delegate: Item {
                // Merkezlemek için genişlik/yüksekliğin yarısını çıkarıyoruz
                x: model.xPos - 6
                y: model.yPos - 6
                width: 12
                height: 12
                z: 50 // Arka planın üstünde, balonların (800) altında kalsın

                // Yanık/Bozulma efekti (Dış Halka)
                Rectangle {
                    anchors.centerIn: parent
                    width: 12
                    height: 12
                    radius: 6
                    color: "#222222"
                    opacity: 0.7
                }

                // Merminin açtığı derin delik (İç Halka)
                Rectangle {
                    anchors.centerIn: parent
                    width: 6
                    height: 6
                    radius: 3
                    color: "black"
                }

                // Derinlik hissi için minik bir parlama (Highlight)
                Rectangle {
                    x: 6; y: 8
                    width: 3; height: 1.5
                    radius: 1
                    color: "white"
                    opacity: 0.3
                }
            }
        }
        // --- SES HAVUZU (SOUND POOL) ---
        // Peş peşe atışlarda seslerin kesilmemesi için 10 adet kopyadan oluşan bir havuz
        Item {
            id: gunshotPoolManager
            property int poolSize: 10
            property int currentIndex: 0

            // Instantiator, görsel olmayan nesneleri (ses gibi) çoklu üretmek için kullanılır
            Instantiator {
                id: gunshotInstantiator
                model: gunshotPoolManager.poolSize
                SoundEffect {
                    source: "qrc:/Orion/assets/sounds/gunshot.wav"
                    volume: 0.8
                }
            }

            // Atış yapıldığında çağrılacak fonksiyon
            function playSound() {
                var sfx = gunshotInstantiator.objectAt(currentIndex)
                if (sfx) {
                    sfx.play()
                }
                // Bir sonraki oynatıcıya geç, sona gelirse başa dön (Round-Robin)
                currentIndex = (currentIndex + 1) % poolSize
            }
        }
        // --- PARÇACIK PATLAMA SİSTEMİ (PARTICLE SYSTEM) ---
        ParticleSystem {
            id: explosionSystem
            anchors.fill: parent
            z: 800 // Balonların üstünde, Hit Marker'ın altında

            // Anlık patlamanın rengini tutacak değişken
            property color sparkColor: "white"

            // 1. Patlama Efekti Vericisi (Emitter)
            Emitter {
                id: explosionEmitter
                group: "sparks"
                emitRate: 0     // Sürekli fışkırtma kapalı, sadece biz 'burst' deyince çalışır
                lifeSpan: 800   // Parçacıkların havada kalma süresi (ms)
                lifeSpanVariation: 200
                size: 16
                sizeVariation: 8
                endSize: 0      // Parçacıklar kaybolurken sönerek küçülsün

                // Hız ve Yön Ayarları (Her yöne rastgele şiddetle fırlasın)
                velocity: AngleDirection {
                    angle: 0
                    angleVariation: 360
                    magnitude: 450 // Patlama şiddeti (Hızı artırdım)
                    magnitudeVariation: 100
                }

                // Yerçekimi efekti (Aşağı doğru düşsünler)
                acceleration: PointDirection { y: 200 }
            }

            // 2. Dışarıdan resim (PNG) İSTEMEYEN, kendi çizdiğimiz parçacık
            ItemParticle {
                groups: ["sparks"]
                delegate: Rectangle {
                    width: 16
                    height: 16
                    radius: 8 // Köşeleri yuvarlatıp tam bir daire yapar
                    color: explosionSystem.sparkColor
                    opacity: 0.8
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: (mouse) => {
                var normX = mouse.x / width
                var normY = mouse.y / height
                gunshotPoolManager.playSound()
                sceneManager.handleInput(normX, normY)
            }
        }

        Rectangle {
            id: hitMarker
            z: 999
            width: 20; height: 20; radius: 10; color: "yellow"
            x: lastHitPoint.x - (width / 2); y: lastHitPoint.y - (height / 2)
            visible: false
            ParallelAnimation {
                id: hitMarkerAnim
                ScriptAction { script: hitMarker.visible = true }
                NumberAnimation { target: hitMarker; property: "opacity"; from: 1.0; to: 0.0; duration: 500 }
                NumberAnimation { target: hitMarker; property: "scale"; from: 1.0; to: 3.0; duration: 500 }
                ScriptAction { script: hitMarker.visible = false }
            }
        }

        Rectangle {
            width: 150; height: 50
            anchors.bottom: parent.bottom; anchors.right: parent.right; anchors.margins: 20
            color: "gray"; radius: 5
            Text { anchors.centerIn: parent; text: "MENÜ (ESC)"; color: "white"; font.bold: true }
            MouseArea {
                anchors.fill: parent
                onClicked: isGameRunning = false
            }
        }

        Rectangle {
            id: announcementOverlay
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.7)
            z: 900
            visible: opacity > 0
            opacity: 0.0

            Text {
                id: announcementLabel
                anchors.centerIn: parent
                text: announcementText
                color: "white"
                font.pixelSize: 80
                font.bold: true
                style: Text.Outline; styleColor: "black"

                SequentialAnimation on scale {
                    loops: Animation.Infinite; running: announcementOverlay.visible
                    NumberAnimation { to: 1.1; duration: 500 }
                    NumberAnimation { to: 1.0; duration: 500 }
                }
            }

            SequentialAnimation {
                id: announcementAnim
                NumberAnimation { target: announcementOverlay; property: "opacity"; to: 1.0; duration: 300 }
                PauseAnimation { duration: 2500 }
                NumberAnimation { target: announcementOverlay; property: "opacity"; to: 0.0; duration: 300 }
            }
        }
        // =========================================================
        // --- OYUN SONU RAPOR EKRANI (GAME OVER REPORT) ---
        // =========================================================
        Rectangle {
            id: reportScreen
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.9)
            z: 1000
            visible: isGameOver

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    text: "SİMÜLASYON RAPORU"
                    color: "orange"
                    font.pixelSize: 60
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 30
                }

                // Rapor Tablosu Arka Planı
                Rectangle {
                    width: selectedPlayerCount === 2 ? 1450 : 900
                    height: 500
                    color: "#050505" // Daha koyu bir ana arka plan
                    //border.color: "orange"
                    //border.width: 2
                    radius: 10
                    Layout.alignment: Qt.AlignHCenter
                    clip: true // İçeriğin köşelerden taşmasını engeller

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 0 // Marginleri sıfırladık, içeriden padding vereceğiz
                        spacing: 0

                        // --- TABLO BAŞLIKLARI (HEADER) ---
                        Rectangle {
                            Layout.fillWidth: true
                            height: 60
                            color: "#151515" // Başlık arka planı biraz daha aydınlık

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 20
                                anchors.rightMargin: 20
                                spacing: 15

                                Text { text: "SEVİYE"; color: "gray"; font.bold: true; font.pixelSize: 18; Layout.preferredWidth: 80 }
                                Text { text: "KAZANAN"; color: "gray"; font.bold: true; font.pixelSize: 18; Layout.preferredWidth: 120; visible: selectedPlayerCount === 2 }
                                Text { text: player1Name + "\n(Skor / Atış / Max Kmb)"; color: "#00FF00"; font.bold: true; font.pixelSize: 18; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                                Text { text: player2Name + "\n(Skor / Atış / Max Kmb)"; color: "cyan"; font.bold: true; font.pixelSize: 18; Layout.fillWidth: true; visible: selectedPlayerCount === 2; horizontalAlignment: Text.AlignHCenter }
                                Text { text: "SÜRE"; color: "yellow"; font.bold: true; font.pixelSize: 18; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight }
                            }
                        }

                        // Kalın Turuncu Ayırıcı Çizgi
                        Rectangle { Layout.fillWidth: true; height: 2; color: "orange" }

                        // --- TUR VERİLERİ (İÇERİK) ---
                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: gameReport ? gameReport.rounds : []
                            clip: true

                            // Delegate: Her bir satırın görünümü
                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 60
                                // Zebra Striping: Çift satırlar açık, tek satırlar koyu siyah
                                color: index % 2 === 0 ? "#121212" : "#0A0A0A"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 20
                                    anchors.rightMargin: 20
                                    spacing: 15

                                    Text {
                                        text: "LVL " + modelData.level;
                                        color: "white"; font.pixelSize: 20; font.bold: true;
                                        Layout.preferredWidth: 80
                                    }

                                    Text {
                                        text: modelData.winner === 1 ? player1Name : player2Name
                                        color: modelData.winner === 1 ? "#00FF00" : "cyan"
                                        font.pixelSize: 20; font.bold: true
                                        Layout.preferredWidth: 120
                                        visible: selectedPlayerCount === 2
                                    }

                                    Text {
                                        text: modelData.p1Score + " P  /  " + modelData.p1Shots + " Atış  /  " + modelData.p1MaxCombo + "x"
                                        color: "white"; font.pixelSize: 20;
                                        Layout.fillWidth: true
                                        horizontalAlignment: Text.AlignHCenter // Tam merkeze hizala
                                    }

                                    Text {
                                        text: modelData.p2Score + " P  /  " + modelData.p2Shots + " Atış  /  " + modelData.p2MaxCombo + "x"
                                        color: "white"; font.pixelSize: 20;
                                        Layout.fillWidth: true
                                        visible: selectedPlayerCount === 2
                                        horizontalAlignment: Text.AlignHCenter // Tam merkeze hizala
                                    }

                                    Text {
                                        text: modelData.timeStr;
                                        color: "yellow"; font.pixelSize: 20;
                                        Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight
                                    }
                                }

                                // İnce Satır Alt Çizgisi
                                Rectangle {
                                    width: parent.width
                                    height: 1
                                    color: "#222222" // Silik gri çizgi
                                    anchors.bottom: parent.bottom
                                }
                            }
                        }
                    }
                }

                // Kapatma Butonu
                Rectangle {
                    width: 300; height: 60
                    color: "orange"
                    radius: 10
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 20
                    Text { anchors.centerIn: parent; text: "MENÜYE DÖN"; color: "black"; font.pixelSize: 24; font.bold: true }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            isGameOver = false
                            isGameRunning = false
                        }
                    }
                }
            }
        }
    }
}
