# Руководство для разработчиков/энтузиастов
Если есть желание что-то поменять (как правило, хочется изменить решатель, не слишком-то он хорош), то следует запастись терпением, роботом EV3, средами разработки Visual Studio 2015 (вообще-то должны и более ранние работать, ничего специфичного там нет), а также платформой Qt 5.6 (опять-таки, версия не слишком важна).
При любом использовании проекта хотелось бы, чтобы ссылки на автора сохранялись хоть в каком-то виде – Пучкин М.В., ИММКН ЮФУ, 2017. Сайт – air.mmcs.sfedu.ru.
## Как с этим всем разбираться
1. Прежде всего, поэкспериментируйте с графическим приложением для Qt. Оно собиралось из-под Qt 5.6 компилятором VS 2015. Вся логика игры описана в файлах «state.cpp» и «state.h» – просчёт эвристики, механизм поиска решения. Алгоритм тут не альфа-бета в чистом виде, а нечто похожее на min-max, где для каждого состояния порождаются дочерние ходы, оцениваются, а родительский узел получает усреднённую оценку с учётом вероятности возникновения плиток разного значения. Состояния хранятся не в виде массива значений на плитках, а в виде степеней двойки, то есть для плитки 2048 в соответствующей ячейке массива будет значение 11.
2. В коде можно изменить настройки скорости анимации плиток – для тестирования можно ставить поменьше время движения и число шагов. Игра зачастую занимает порядка 3-4 тысяч ходов.
3. При любых изменениях «state.cpp» и «state.h» постарайтесь сохранить сигнатуры основных функций – это понадобится для создания DLL. Если есть сомнения – загляните во вложенную папку, там проект динамической библиотеки (Visual Studio). Этот проект подхватывает указанные файлы из внешней папки, поэтому ничего особо копировать никуда не надо, просто запустить его и собрать dll.
4. Собранную dll (в режиме Release, для x86) скопировать в соответствующую папку приложения FinalDesktopProject, и там уже экспериментировать с роботом EV3.

Удачи!