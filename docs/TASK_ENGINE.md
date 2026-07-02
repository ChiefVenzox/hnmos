# Task Engine

## Hedef

Task Engine, HNShell'den gelen kullanici niyetini izlenebilir, izinlenebilir ve ileride AI Runtime Interface tarafindan yorumlanabilir gorev kayitlarina ceviren HNMos Core OS katmanidir.

Task Engine kernel icine gomulu AI degildir. Ilk tasarimda HNShell ile AI Runtime Interface arasindaki gorev yonlendirme katmanidir.

## Dosya Yapisi

```text
ai/
`-- task_engine/
    |-- task_engine.c
    |-- task_engine.h
    |-- task_request.h
    |-- task_result.h
    `-- task_state.h
```

Bu dosyalar CODEX-06 hedef yapisidir; uygulama sonraki kademede baslayabilir.

## Task Engine Mantigi

Task Engine'in temel sorusu:

```text
Kullanicinin komutu sistemde hangi izlenebilir goreve karsilik geliyor?
```

Ornek:

```text
hn system inspect memory
  -> task.kind = system.inspect
  -> task.target = memory
  -> permission = observe
  -> route = Core OS memory summary
```

## Gorev Akisi

```text
HNShell command
  -> parser argv
  -> task router
  -> task request
  -> safety classification
  -> execution route
  -> task result
  -> event/audit record
```

Bu akis, AI'nin serbest metinle dogrudan sistem komutu calistirmasini engeller.

## Task Request Taslagi

Ilk task request alanlari:

```text
task_id
source
command_name
argv
intent
target
permission_level
context_policy
state
```

`source` degerleri:

- HNShell.
- Kernel console.
- Future user process.
- Future AI runtime.

`permission_level` degerleri:

- Observe.
- Suggest.
- Draft.
- Request.
- Execute.

Ilk varsayilan `Observe` veya `Suggest` olmalidir.

## Task State

Ilk state modeli:

```text
Created
Classified
WaitingForContext
WaitingForPermission
Ready
Running
Completed
Denied
Failed
```

Scheduler task state ile karistirilmamalidir. Scheduler CPU zamanini yonetir; Task Engine ise OS seviyesinde niyet ve is takibini yonetir.

## Kernel Task Skeleton ile Iliski

HNMOS-CODEX-11 ile `kernel01/kernel/task/` altinda ayri bir kernel task registry ve cooperative scheduler skeleton eklenmistir. Bu katman CPU/task zamanlama temelidir.

Task Engine ise hala kullanici niyeti, izin, audit ve AI Runtime Interface yonlendirmesi icin planlanan daha ust seviye katmandir. Bu iki model bilincli olarak ayridir:

```text
kernel task scheduler -> internal execution bookkeeping
task engine           -> user intent and audited work records
```

Mevcut kernel task skeleton userspace program calistirmaz ve dosya sistemindeki dosyalari executable olarak yuklemez.

## Built-in Komutlarla Iliski

Ilk HNShell komutlari Task Engine'e su sekilde baglanabilir:

```text
hn status                -> system.status
hn system inspect memory -> system.inspect.memory
hn build info            -> system.build.info
hn task list             -> task.list
hn event list            -> event.list
hn ai status             -> ai.status
```

`hn ai status`, model calistirmaz. Sadece AI Runtime Interface durumunu sorgulayan observe seviyesinde bir gorevdir.

## AI Runtime Interface ile Iliski

Task Engine AI Runtime Interface'e iki tip istek gonderebilir:

- Oneri istegi.
- Baglam ozetleme istegi.

Task Engine, AI'den gelen cevabi dogrudan calistirmaz. Cevap once Safety Layer ve izin politikasindan gecer.

## Event ve Audit Mantigi

Her task kaydi olay uretmelidir:

```text
task.created
task.classified
task.permission.requested
task.denied
task.completed
task.failed
```

Audit kaydi gerektiren durumlar:

- Permission seviyesi `Request` veya `Execute`.
- AI tarafindan uretilen oneriler.
- Kalici sistem degisikligi isteyen komutlar.
- Dosya sistemi veya driver event katmanina etki eden komutlar.

## AI'nin Dogrudan Komut Calistirmamasi

Task Engine'in ana guvenlik ilkesi:

```text
AI response is not execution.
```

AI cevabi sadece task proposal, draft veya explanation olarak kabul edilir. Sistem komutu ancak HNShell/HNLang/syscall-backed OS service uzerinden, izin ve audit kaydiyla calisir.

## Ilk Stub Davranisi

Gercek Task Engine henuz yokken ilk stub:

- `hn task list` icin `task engine: not loaded` dondurur.
- `hn ai status` icin AI Runtime stub'una observe seviyesinde istek hazirlar.
- Her task icin sabit bir `task_id` veya artan sayaç planlar.

## Tamamlanma Kriterleri

- Task Engine'in HNShell ile AI Runtime arasindaki yeri tanimlanmistir.
- Task request, result ve state alanlari yazilmistir.
- Built-in HNShell komutlarinin task karsiliklari listelenmistir.
- AI cevabinin dogrudan execution olmadigi ilke olarak kayda gecmistir.

## HNMOS-CODEX-07 HNLang Baglantisi

CODEX-07'de HNLang task script'leri Task Engine'e baglanacak sekilde tasarlanir:

1. `task "name" { ... }` bloklari task request taslagina cevrilir.
2. `kernel.inspect(...)` ifadeleri observe seviyesinde sistem sorgusu olur.
3. `ai.ask(...)` ifadeleri AI Runtime Interface'e draft/request olarak gider.
4. `shell.run(...)` ifadeleri HNShell komut taslagina cevrilir.
5. Runtime execute etmeden once Safety Layer ve izin akisi beklenir.
