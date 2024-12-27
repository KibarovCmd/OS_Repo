# Linux Shell Uygulaması Proje Raporu

## Projenin Amacı

Bu proje, Linux işletim sisteminde çalışan basit bir kabuk (shell) uygulamasının gerçekleştirilmesini amaçlamaktadır. Kabuk, kullanıcı ile işletim sistemi arasında bir arayüz görevi görerek, kullanıcının komutlarını yorumlayan ve işletim sistemine ileten bir programdır.

## Projenin Kapsamı

Uygulama aşağıdaki temel özellikleri desteklemektedir:

1. **Temel Komut Çalıştırma**
   - Linux sistem komutlarını çalıştırabilme (ls, pwd, echo vb.)
   - Komut argümanlarını işleyebilme
   - Hata durumlarını yönetebilme

2. **Giriş/Çıkış Yönlendirme**
   - Giriş yönlendirme (`<`) ile dosyadan okuma
   - Çıkış yönlendirme (`>`) ile dosyaya yazma
   - Yönlendirme hatalarını uygun şekilde raporlama

3. **Boru Hattı İşlemleri**
   - İki veya daha fazla komutu boru hattı (`|`) ile bağlama
   - Komutlar arası veri akışını yönetme
   - Çoklu boru hattı işlemlerini destekleme

4. **Arka Plan İşlemleri**
   - Komutları arka planda çalıştırabilme (`&`)
   - Arka plan işlem durumlarını takip etme
   - İşlem tamamlandığında PID ve çıkış kodunu raporlama

5. **Çoklu Komut Desteği**
   - Noktalı virgül (`;`) ile ayrılmış komutları işleme
   - Her bir komutu sırayla ve bağımsız olarak çalıştırma

## Teknik Detaylar

### Proje Yapısı

Proje, modüler bir yapıda organize edilmiş olup şu bileşenlerden oluşmaktadır:

1. **Ana Program (main.c)**
   - Programın giriş noktası
   - Kullanıcı arayüzü
   - Sinyal yönetimi

2. **Kabuk Çekirdeği (shell.c)**
   - Komut yürütme mantığı
   - Süreç yönetimi
   - Boru hattı işlemleri

3. **Komut Ayrıştırıcı (parser.c)**
   - Komut satırı ayrıştırma
   - Argüman işleme
   - Yönlendirme ve boru hattı tokenlarını ayırma

### Kullanılan Sistem Çağrıları

Proje, aşağıdaki Linux sistem çağrılarını kullanmaktadır:

- `fork()`: Yeni süreç oluşturma
- `exec()`: Komut çalıştırma
- `pipe()`: Boru hattı oluşturma
- `dup2()`: Dosya tanımlayıcı yönlendirme
- `wait()`: Süreç bekletme
- `signal()`: Sinyal yönetimi

### Bellek Yönetimi

Proje, dinamik bellek yönetimi için şu stratejileri kullanmaktadır:

- Dinamik dizi tahsisi ve serbest bırakma
- Bellek sızıntılarını önlemek için sistematik temizleme
- Güvenli bellek erişimi için sınır kontrolleri

## Kullanım Örnekleri

1. **Temel Komut Çalıştırma**
   ```bash
   > ls -l
   > pwd
   > echo "Merhaba Dünya"
   ```

2. **Dosya Yönlendirme**
   ```bash
   > echo "test" > dosya.txt
   > cat < dosya.txt
   > ls -l > liste.txt
   ```

3. **Boru Hattı Kullanımı**
   ```bash
   > ls | grep .txt
   > cat dosya.txt | sort | uniq
   ```

4. **Arka Plan İşlemleri**
   ```bash
   > sleep 5 &
   > long_running_command &
   ```

5. **Çoklu Komutlar**
   ```bash
   > echo "ilk"; echo "ikinci"; echo "üçüncü"
   ```

## Hata Yönetimi

Program şu hata durumlarını ele almaktadır:

1. Dosya işlemleri hataları
   - Dosya bulunamadı
   - Erişim izni yok
   - Okuma/yazma hataları

2. Komut çalıştırma hataları
   - Komut bulunamadı
   - Argüman hataları
   - Yetersiz izinler

3. Sistem kaynakları hataları
   - Bellek yetersizliği
   - Maksimum süreç sayısı aşımı
   - Dosya tanımlayıcı limiti aşımı

## Sonuç

Bu proje, Linux işletim sistemi prensiplerini ve sistem programlama kavramlarını uygulamalı olarak öğrenmek için önemli bir örnek teşkil etmektedir. Projenin geliştirilmesi sürecinde:

- Süreç yönetimi
- Inter-process iletişimi
- Dosya işlemleri
- Sinyal yönetimi
- Bellek yönetimi

gibi temel sistem programlama konuları pratik edilmiştir.

Proje, temel shell işlevselliğini başarıyla gerçekleştirmekte ve geliştirilmeye açık bir temel oluşturmaktadır.