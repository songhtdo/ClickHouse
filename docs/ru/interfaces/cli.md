# Клиент командной строки

Для работы из командной строки вы можете использовать `clickhouse-client`:

```bash
$ clickhouse-client
ClickHouse client version 0.0.26176.
Connecting to localhost:9000.
Connected to ClickHouse server version 0.0.26176.

:)
```

Клиент поддерживает параметры командной строки и конфигурационные файлы. Подробнее читайте в разделе "[Конфигурирование](#interfaces_cli_configuration)". 

## Использование {#cli_usage}

Клиент может быть использован в интерактивном и неинтерактивном (batch) режиме.
Чтобы использовать batch режим, укажите параметр query, или отправьте данные в stdin (проверяется, что stdin - не терминал), или и то, и другое.
Аналогично HTTP интерфейсу, при использовании одновременно параметра query и отправке данных в stdin, запрос составляется из конкатенации параметра query, перевода строки, и данных в stdin. Это удобно для больших INSERT запросов.

Примеры использования клиента для вставки данных:

```bash
$ echo -ne "1, 'some text', '2016-08-14 00:00:00'\n2, 'some more text', '2016-08-14 00:00:01'" | clickhouse-client --database=test --query="INSERT INTO test FORMAT CSV";

$ cat <<_EOF | clickhouse-client --database=test --query="INSERT INTO test FORMAT CSV";
3, 'some text', '2016-08-14 00:00:00'
4, 'some more text', '2016-08-14 00:00:01'
_EOF

$ cat file.csv | clickhouse-client --database=test --query="INSERT INTO test FORMAT CSV";
```

В batch режиме в качестве формата данных по умолчанию используется формат TabSeparated. Формат может быть указан в секции FORMAT запроса.

По умолчанию, в batch режиме вы можете выполнить только один запрос. Чтобы выполнить несколько запросов из "скрипта", используйте параметр --multiquery. Это работает для всех запросов кроме INSERT. Результаты запросов выводятся подряд без дополнительных разделителей.
Также, при необходимости выполнить много запросов, вы можете запускать clickhouse-client на каждый запрос. Заметим, что запуск программы clickhouse-client может занимать десятки миллисекунд.

В интерактивном режиме, вы получите командную строку, в которую можно вводить запросы.

Если не указано multiline (по умолчанию):
Чтобы выполнить запрос, нажмите Enter. Точка с запятой на конце запроса не обязательна. Чтобы ввести запрос, состоящий из нескольких строк, перед переводом строки, введите символ обратного слеша: `\` - тогда после нажатия Enter, вам предложат ввести следующую строку запроса.

Если указано multiline (многострочный режим):
Чтобы выполнить запрос, завершите его точкой с запятой и нажмите Enter. Если в конце введённой строки не было точки с запятой, то вам предложат ввести следующую строчку запроса.

Исполняется только один запрос, поэтому всё, что введено после точки с запятой, игнорируется.

Вместо или после точки с запятой может быть указано `\G`. Это обозначает использование формата Vertical. В этом формате каждое значение выводится на отдельной строке, что удобно для широких таблиц. Столь необычная функциональность добавлена для совместимости с MySQL CLI.

Командная строка сделана на основе readline (и history) (или libedit, или без какой-либо библиотеки, в зависимости от сборки) - то есть, в ней работают привычные сочетания клавиш, а также присутствует история.
История пишется в `~/.clickhouse-client-history`.

По умолчанию, в качестве формата, используется формат PrettyCompact (красивые таблички). Вы можете изменить формат с помощью секции FORMAT запроса, или с помощью указания `\G` на конце запроса, с помощью аргумента командной строки `--format` или `--vertical`, или с помощью конфигурационного файла клиента.

Чтобы выйти из клиента, нажмите Ctrl+D (или Ctrl+C), или наберите вместо запроса одно из: "exit", "quit", "logout", "учше", "йгше", "дщпщге", "exit;", "quit;", "logout;", "учшеж", "йгшеж", "дщпщгеж", "q", "й", "q", "Q", ":q", "й", "Й", "Жй"

При выполнении запроса, клиент показывает:

1. Прогресс выполнение запроса, который обновляется не чаще, чем 10 раз в секунду (по умолчанию). При быстрых запросах, прогресс может не успеть отобразиться.
2. Отформатированный запрос после его парсинга - для отладки.
3. Результат в заданном формате.
4. Количество строк результата, прошедшее время, а также среднюю скорость выполнения запроса.

Вы можете прервать длинный запрос, нажав Ctrl+C. При этом вам всё равно придётся чуть-чуть подождать, пока сервер остановит запрос. На некоторых стадиях выполнения, запрос невозможно прервать. Если вы не дождётесь и нажмёте Ctrl+C второй раз, то клиент будет завершён.

Клиент командной строки позволяет передать внешние данные (внешние временные таблицы) для использования запроса. Подробнее смотрите раздел "Внешние данные для обработки запроса"

### Запросы с параметрами {#cli-queries-with-parameters}

Вы можете создать запрос с параметрами и передавать в них значения из приложения. Это позволяет избежать форматирования запросов на стороне клиента, если известно, какие из параметров запроса динамически меняются. Например:

```bash
clickhouse-client --param_parName="[1, 2]"  -q "SELECT * FROM table WHERE a = {parName:Array(UInt16)}"
```

#### Cинтаксис запроса {#cli-queries-with-parameters-syntax}

Отформатируйте запрос обычным способом. Представьте значения, которые вы хотите передать из параметров приложения в запрос в следующем формате:

```sql
{<name>:<data type>}
```

- `name` — идентификатор подстановки. В консольном клиенте его следует использовать как часть имени параметра `--param_<name> = value`.
- `data type` — [тип данных](../data_types/index.md) значения. Например, структура данных `(integer, ('string', integer))` может иметь тип данных `Tuple(UInt8, Tuple(String, UInt8))` ([целочисленный](../data_types/int_uint.md) тип может быть и другим).

#### Пример

```bash
$ clickhouse-client --param_tuple_in_tuple="(10, ('dt', 10))" -q "SELECT * FROM table WHERE val = {tuple_in_tuple:Tuple(UInt8, Tuple(String, UInt8))}"
```

## Конфигурирование {#interfaces_cli_configuration}

В `clickhouse-client` можно передавать различные параметры (все параметры имеют значения по умолчанию) с помощью:

- Командной строки.

   Параметры командной строки переопределяют значения по умолчанию и параметры конфигурационных файлов.

- Конфигурационных файлов.

   Параметры в конфигурационных файлах переопределяют значения по умолчанию.

### Параметры командной строки

- `--host, -h` — имя сервера, по умолчанию — localhost. Вы можете использовать как имя, так и IPv4 или IPv6 адрес.
- `--port` — порт, к которому соединяться, по умолчанию — 9000. Замечу, что для HTTP и родного интерфейса используются разные порты. 
- `--user, -u` — имя пользователя, по умолчанию — default.
- `--password` — пароль, по умолчанию — пустая строка.  
- `--query, -q` — запрос для выполнения, при использовании в неинтерактивном режиме.
- `--database, -d` — выбрать текущую БД, по умолчанию — текущая БД из настроек сервера (по умолчанию — БД default).
- `--multiline, -m` — если указано — разрешить многострочные запросы, не отправлять запрос по нажатию Enter.
- `--multiquery, -n` — если указано — разрешить выполнять несколько запросов, разделённых точкой с запятой. Работает только в неинтерактивном режиме.
- `--format, -f` — использовать указанный формат по умолчанию для вывода результата.
- `--vertical, -E` — если указано, использовать формат Vertical по умолчанию для вывода результата. То же самое, что --format=Vertical. В этом формате каждое значение выводится на отдельной строке, что удобно для отображения широких таблиц.
- `--time, -t` — если указано, в неинтерактивном режиме вывести время выполнения запроса в stderr.
- `--stacktrace` — если указано, в случае исключения, выводить также его стек трейс.
- `--config-file` — имя конфигурационного файла.
- `--secure` — если указано, будет использован безопасный канал.
- `--param_<name>` — значение параметра для [запроса с параметрами](#cli-queries-with-parameters).

### Конфигурационные файлы

`clickhouse—client` использует первый существующий файл из:

- Определенного параметром `--config-file`.
- `./clickhouse-client.xml`
- `~/.clickhouse-client/config.xml`
- `/etc/clickhouse-client/config.xml`

Пример конфигурационного файла:

```xml
<config>
    <user>username</user>
    <password>password</password>
    <secure>False</secure>
</config>
```
[Оригинальная статья](https://clickhouse.yandex/docs/ru/interfaces/cli/) <!--hide-->
