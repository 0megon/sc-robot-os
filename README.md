# sc-robot-os
Основной целью разработки программного комплекса являлось создание надежной и эффективной системы управления робототехническим комплексом, обеспечивающей выполнение различных задач, таких как движение робота, обнаружение препятствий, контроль заряда батареи и другие.  

Язык программирования C++ выбран для реализации основной части программного комплекса благодаря его высокой производительности, эффективности в использовании аппаратных ресурсов и возможности работы с низкоуровневыми аппаратными интерфейсами. Для создания веб-сервера был использован Python из-за его простоты в реализации веб-приложений и JavaScript для функционала веб-интерфейса. 

Изначально планировалось использовать ESP32 в качестве главного контроллера робототехнического комплекса. Однако для реализации всего функционала потребовалась несколько большая способность к многопоточной работе, чем может предложить ESP32. Поэтому в качестве главного контроллера был взят микрокомпьютер Raspberry Pi 5. Библиотека WiringPi была использована используется для взаимодействия с аппаратной частью микрокомпьютера Raspberry Pi 5, включая управление двигателями и считывание данных с сенсоров. 

Пользователь способен подключаться к комплексу по веб-интерфейсу и управлять им напрямую. 

Веб-сервер, написанный на Python, обрабатывает GET-запросы и отправляет HTML-страницу, подключившемуся пользователю, который находится в той же подсети. HTML-страница содержит форму для ввода команд и скрипт для взаимодействия с сервером через WebSocket. Веб-сервер также обрабатывает POST-запросы, посылающиеся через формы на странице веб-интерфейса и содержащие команды для робототехнического комплекса. Команды записываются в файл, который затем считывается основной программой управления комплексом. На данный функционал доступен пользователю по выбору: возможно либо начать работу по заранее заданному алгоритму, либо управлять комплексом напрямую. 

Помимо основной системы контроля на C++ были написаны небольшие программы для обработки сигналов с периферийных устройств. Взаимодействие этих программ и веб-сервера с основной программой выполняется посредством промежуточных файлов, в которые производится запись и чтение значений. 
