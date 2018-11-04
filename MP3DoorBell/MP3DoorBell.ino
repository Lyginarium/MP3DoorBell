// MP3DoorBell v. 0.1 "it's alive"
// Lyginarium 2018 (c)
//
// Дверной звонок, воспроизводящий мелодии формата MP3 с microSD карты, с 
// некоторыми дополнительными фичами.
// Проект построен на плате Arduino Nano v.3 и плате MP3-TP-16P, являющейся
// дешевым клоном DFPlayer от DFRobot.
// Мелодии могут "разбавляться" забавными и полезными голосовыми вставками,
// т. н. "рекламой", также записаной в формате MP3 на microSD-карту.
//
// Для работы проекта необходимио создать на карте памяти две папки:
// 01 - для мелодий звонка; формат имени файла - 001.mp3, 002.mp3, ...
// и т. д., до 999 файлов в одной папке.
//
// Папку advert для "рекламы"; формат имени файла вида 0001.mp3 и т. д.
//
//Для успешной сборки проекта в среде разработки Arduino должны
// быть установленной библиотеки SoftwareSerial и DFPlayer Mini Mp3 by Makuna.

#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

// реализация класса оповещений, методами которого можно пользоваться для отладки
// или вывода информации о проигрываемых файлах и ошибках через последовательный порт.


class Mp3Notify
{
public:
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnCardOnline(uint16_t code)
  {
    Serial.println("Card online ");
  }
  static void OnCardInserted(uint16_t code)
  {
    Serial.println("Card inserted ");
  }
  static void OnCardRemoved(uint16_t code)
  {
    Serial.println("Card removed ");
  }
};
 //Конец реализации класса оповещений.



// Поскольку встроенный в Ардуину аппаратный последовательный интерфес нам может понадобится для отладки,
// создаем экземпляр класса программно эмулированного последовательного интерфейса
SoftwareSerial secondarySerial(10, 11); // RX, TX

// Создаем экземпляр класса DFMiniMp3, 
// определенного с указанным выше классом уведомлений и классом последовательного интерфейса.
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

uint32_t lastAdvert; // длительность проигрывания трека без рекламы
uint32_t lastPlay; // длительность проигрывания трека
uint16_t volumeTmp;
bool ReedRelayClose; // флаг состояни геркона: 1 - замкнут (дверь закрыта), 0 - разомкнут (дверь открыта)
const int PlayButton = 2; // кнопка звонка: нажата/отпущена
const int MP3ModuleBusy = 3; // состояние МП3-плеера: занят/свободен
const int DoorLimitSwitch = 4; // геркон
const int PlaybackLED = 5; // индикация состояния МП3-плеера: занят/свободен

void setup() 
{
  pinMode (PlayButton, INPUT);
  pinMode (MP3ModuleBusy, INPUT);
  pinMode (DoorLimitSwitch, INPUT);
  pinMode (PlaybackLED, OUTPUT);
  
  //Serial.begin(115200);
  //Serial.println("initializing..."); 
  
  mp3.begin(); // инициация, х. з. чо это...
 // uint16_t volume = mp3.getVolume(); // лишнее, т. к. при включении громкость модуль сам выставляет себе на максимум, т. е. 30
  
  //Serial.print("volume was ");
  //Serial.println(volume);
  
  mp3.setVolume(10);
  
 // volume = mp3.getVolume();
 // Serial.print(" and changed to  ");
 // Serial.println(volume);
  
  //Serial.println("track 1 from folder 1"); 
  //mp3.playFolderTrack(1, 1); // sd:/01/001.mp3

  
  
}

void loop() 
{
  if ((digitalRead(PlayButton) ==  HIGH)&& // Если нажата кнопка звонка и ничего не проигрывается, то
    (digitalRead(MP3ModuleBusy) == HIGH))  // 
    {
      ReedRelayClose = digitalRead(DoorLimitSwitch); // запомнить, в каком состоянии геркон
     // Serial.println("track 1 from folder 1"); 
      digitalWrite(PlaybackLED, HIGH); // зажечь индикаторный светодиод
      volumeTmp = mp3.getVolume(); // запомнить громкость
      mp3.setVolume(0); // убавить громкость до 0
  mp3.playFolderTrack(1, random(1, 28)); // папка 01, случайный трек от 1 до 27 включительно
    
      for(int i = 1; i <= volumeTmp; i++) // плавно вывести громкость вверх до установленной величины
        {
        mp3.setVolume(i);
        delay(100);
        }
        
lastAdvert = millis();
  lastPlay = millis();
 
    }
    uint32_t now = millis(); // время с начала выполнения программы, мс
    
  if (((now - lastAdvert) > 25000)&&(digitalRead(MP3ModuleBusy) == LOW)) // если прошло больше 25 с без рекламы
  {
    // interrupt the song and play the advertisement, it will
    // return to the song when its done playing automatically
    mp3.playAdvertisement(random(1, 10)); // играть случайную рекламную вставку с 1 по 9 файл включительно
    lastAdvert = millis();
  }
  
  if ((((now - lastPlay) > 90000)&&(digitalRead(MP3ModuleBusy) == LOW)) || ((digitalRead(DoorLimitSwitch) == LOW) && (ReedRelayClose)))
  // если трек играет больше 90 секунд или дверь была закрыта и открылась
  {
    digitalWrite (PlaybackLED, LOW); // гасим индикатор активности плеера
    volumeTmp = mp3.getVolume();
      
    
      for(int i = (volumeTmp - 1); i >= 0; i--) // плавно убавляем звук до нуля
        {
        mp3.setVolume(i);
        delay(100);
        }
    mp3.stop();
    mp3.setVolume(volumeTmp);
    //lastPlay = now;
    //lastAdvert = now;
  }
  
  mp3.loop(); // аптека, улица, фонарь
  }
  
