import { assert } from "chai";
import { ClientData } from "../src/clientData.js";
import { Value, Text, Log } from "../src/data.js";
import { Func, AnonymousFunc } from "../src/func.js";
import { valType } from "../src/message.js";
import { View } from "../src/view.js";
import { Field, FieldBase } from "../src/field.js";
import { Member } from "../src/member.js";
import { eventType } from "../src/event.js";

describe("Member Tests", function () {
  const selfName = "test";
  let data: ClientData;
  const member = (member: string) => new Member(new Field(data, member));
  beforeEach(function () {
    data = new ClientData(selfName, () => undefined);
  });
  describe("#name", function () {
    it("returns member name", function () {
      assert.strictEqual(member("a").name, "a");
    });
  });
  describe("#value()", function () {
    it("returns Value object", function () {
      const v = member("a").value("b");
      assert.instanceOf(v, Value);
      assert.strictEqual(v.member.name, "a");
      assert.strictEqual(v.name, "b");
    });
  });
  describe("#text()", function () {
    it("returns Text object", function () {
      const v = member("a").text("b");
      assert.instanceOf(v, Text);
      assert.strictEqual(v.member.name, "a");
      assert.strictEqual(v.name, "b");
    });
  });
  describe("#view()", function () {
    it("returns View object", function () {
      const v = member("a").view("b");
      assert.instanceOf(v, View);
      assert.strictEqual(v.member.name, "a");
      assert.strictEqual(v.name, "b");
    });
  });
  describe("#func()", function () {
    it("returns Func object", function () {
      const v = member("a").func("b");
      assert.instanceOf(v, Func);
      assert.strictEqual(v.member.name, "a");
      assert.strictEqual(v.name, "b");
    });
    it("returns AnonymousFunc object in which callback is already set", function () {
      const v = member(selfName).func(() => undefined, valType.none_, []);
      assert.instanceOf(v, AnonymousFunc);
      assert.strictEqual(v.base_?.member?.name, selfName);
      assert.isNotEmpty(v.base_?.name || "");
      assert.isNotEmpty(data.funcStore.dataRecv.get(selfName) || new Map());
    });
  });
  describe("#log()", function () {
    it("returns log object", function () {
      const v = member("a").log();
      assert.instanceOf(v, Log);
      assert.strictEqual(v.member.name, "a");
    });
  });
  describe("#values()", function () {
    it("returns list of entries in data.valueStore.entry", function () {
      data.valueStore.entry.set("a", ["b", "c", "d"]);
      assert.isArray(member("a").values());
      assert.lengthOf(member("a").values(), 3);
    });
    it("returns emmpty array by default", function () {
      assert.isArray(member("a").values());
      assert.isEmpty(member("a").values());
    });
  });
  describe("#texts()", function () {
    it("returns list of entries in data.textStore.entry", function () {
      data.textStore.entry.set("a", ["b", "c", "d"]);
      assert.isArray(member("a").texts());
      assert.lengthOf(member("a").texts(), 3);
    });
    it("returns emmpty array by default", function () {
      assert.isArray(member("a").texts());
      assert.isEmpty(member("a").texts());
    });
  });
  describe("#views()", function () {
    it("returns list of entries in data.viewStore.entry", function () {
      data.viewStore.entry.set("a", ["b", "c", "d"]);
      assert.isArray(member("a").views());
      assert.lengthOf(member("a").views(), 3);
    });
    it("returns emmpty array by default", function () {
      assert.isArray(member("a").views());
      assert.isEmpty(member("a").views());
    });
  });
  describe("#funcs()", function () {
    it("returns list of entries in data.funcStore.entry", function () {
      data.funcStore.entry.set("a", ["b", "c", "d"]);
      assert.isArray(member("a").funcs());
      assert.lengthOf(member("a").funcs(), 3);
    });
    it("returns emmpty array by default", function () {
      assert.isArray(member("a").funcs());
      assert.isEmpty(member("a").funcs());
    });
  });
  describe("#onValueEntry", function () {
    it("handles value entry event", function () {
      let called = 0;
      member("a").onValueEntry.on(() => ++called);
      data.eventEmitter.emit(eventType.valueEntry(new FieldBase("a")));
    });
  });
  describe("#onTextEntry", function () {
    it("handles text entry event", function () {
      let called = 0;
      member("a").onTextEntry.on(() => ++called);
      data.eventEmitter.emit(eventType.textEntry(new FieldBase("a")));
    });
  });
  describe("#onFuncEntry", function () {
    it("handles func entry event", function () {
      let called = 0;
      member("a").onFuncEntry.on(() => ++called);
      data.eventEmitter.emit(eventType.funcEntry(new FieldBase("a")));
    });
  });
  describe("#onViewEntry", function () {
    it("handles view entry event", function () {
      let called = 0;
      member("a").onViewEntry.on(() => ++called);
      data.eventEmitter.emit(eventType.viewEntry(new FieldBase("a")));
    });
  });
  describe("#onSync", function () {
    it("handles sync event", function () {
      let called = 0;
      member("a").onSync.on(() => ++called);
      data.eventEmitter.emit(eventType.sync(new FieldBase("a")));
    });
  });
});
