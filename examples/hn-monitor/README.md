# hn-monitor

`hn-monitor`, HNMos sistem durumunu, public hardware event snapshot'larini ve task durumlarini gosteren monitor uygulamasi hedefidir.

Ilk hedef:

- `read.system_status` izni ile kernel/system ozetini okumak.
- `read.events.public` izni ile Driver Event Layer snapshot'larini gostermek.
- Visual Task Monitor'a veri saglayacak user-space app modelini test etmek.

Calistirma hedefi:

```text
hn app run hn-monitor
```
