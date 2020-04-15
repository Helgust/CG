ComputerGraphics
Задача, написать простой рейтрейсер.
Базовая Часть :
Необходимо реализовать локальное освещение по модели фонга или другим анало-
гичным моделям.
∙ Необходимо реализовать тени.
∙ Необходимо реализовать зеркальные отражения.
∙ Необходимо использовать минимум 3 различных материала.
∙ Необходимо использовать минимум 2 различных примитива (например, треугольник
и сфера).
∙ В сцене должен быть хотя бы 1 источник света.
∙ Рендеринг одного изображения не должен занимать больше 1 секунды для трасси-
ровки лучей (для процессора уровня 6-ядернорго AMD Ryzen 5 3600 или GPU уровня
Nvidia GTX1070).


 Базовая Часть +15

 Функциональные элементы:
 Текстура "Шашечка" +1
 Дополнительные геометрические примитивы +2 
 Устранение ступенчатости +1 
 Преломления +1
 Сферическая карта+1


 Скорость:
 Использование многопоточности +2
 
 
 
 ЗАПУСК:
Запуск Вашей программы должен выглядить следующим образом:
. / r t −out <output_path> −s c ene <scene_number> −thr eads <threads>
∙ output_path - путь к выходному изображению (относительный).
∙ scene_number - номер сцены от 1 до 3.

Порядок компиляции:
mkdir bui ld
cd bui ld
cmake −DCMAKE\_BUILD\_TYPE=Release ..
make −j 4

Делать лучше из под Linux
