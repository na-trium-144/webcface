import { assert } from "chai";
import { ClientData } from "../src/clientData.js";
import { Value, Text, Log } from "../src/data.js";
import { Field } from "../src/field.js";
import { Member } from "../src/member.js";

describe("Value Tests", function () {
  const selfName = "test";
  let data: ClientData;
  const value = (member: string, field: string) =>
    new Value(new Field(data, member, field));
  beforeEach(function () {
    data = new ClientData(selfName, () => undefined);
  });
  describe("#member", function () {
    it("returns Member object with its member name", function () {
      assert.instanceOf(value("a", "b").member, Member);
      assert.strictEqual(value("a", "b").member.name, "a");
    });
  });
  describe("#name", function () {
    it("returns field name", function () {
      assert.strictEqual(value("a", "b").name, "b");
    });
  });
  describe("#child()", function () {
    it("returns child Value object", function () {
      const c = value("a", "b").child("c");
      assert.instanceOf(c, Value);
      assert.strictEqual(c.member.name, "a");
      assert.strictEqual(c.name, "b.c");
    });
  });
  describe("#tryGet()", function () {
    it("returns null by default", function () {
      assert.isNull(value("a", "b").tryGet());
    });
    it("returns first element of value if data.valueStore.dataRecv is set", function () {
      data.valueStore.dataRecv.set("a", new Map([["b", [2, 3, 4]]]));
      assert.strictEqual(value("a", "b").tryGet(), 2);
    });
    it("sets request when member is not self name", function () {
      value("a", "b").tryGet();
      assert.strictEqual(data.valueStore.req.get("a")?.get("b"), 1);
    });
    it("does not set request when member is self name", function () {
      value(selfName, "b").tryGet();
      assert.isEmpty(data.valueStore.req);
    });
  });
  describe("#get()", function () {
    it("returns 0 by default", function () {
      assert.strictEqual(value("a", "b").get(), 0);
    });
    it("returns first element of value if data.valueStore.dataRecv is set", function () {
      data.valueStore.dataRecv.set("a", new Map([["b", [2, 3, 4]]]));
      assert.strictEqual(value("a", "b").get(), 2);
    });
    it("sets request when member is not self name", function () {
      value("a", "b").get();
      assert.strictEqual(data.valueStore.req.get("a")?.get("b"), 1);
    });
    it("does not set request when member is self name", function () {
      value(selfName, "b").get();
      assert.isEmpty(data.valueStore.req);
    });
  });
  describe("#tryGetVec()", function () {
    it("returns null by default", function () {
      assert.isNull(value("a", "b").tryGetVec());
    });
    it("returns value if data.valueStore.dataRecv is set", function () {
      data.valueStore.dataRecv.set("a", new Map([["b", [2, 3, 4]]]));
      assert.sameOrderedMembers(value("a", "b").tryGetVec() || [], [2, 3, 4]);
    });
    it("sets request when member is not self name", function () {
      value("a", "b").tryGetVec();
      assert.strictEqual(data.valueStore.req.get("a")?.get("b"), 1);
    });
    it("does not set request when member is self name", function () {
      value(selfName, "b").tryGetVec();
      assert.isEmpty(data.valueStore.req);
    });
  });
  describe("#getVec()", function () {
    it("returns empty array by default", function () {
      assert.isArray(value("a", "b").getVec());
      assert.isEmpty(value("a", "b").getVec());
    });
    it("returns value if data.valueStore.dataRecv is set", function () {
      data.valueStore.dataRecv.set("a", new Map([["b", [2, 3, 4]]]));
      assert.sameOrderedMembers(value("a", "b").getVec(), [2, 3, 4]);
    });
    it("sets request when member is not self name", function () {
      value("a", "b").getVec();
      assert.strictEqual(data.valueStore.req.get("a")?.get("b"), 1);
    });
    it("does not set request when member is self name", function () {
      value(selfName, "b").getVec();
      assert.isEmpty(data.valueStore.req);
    });
  });
  describe("#set()", function () {
    it("sets array of single value when single value is passed", function () {
      value(selfName, "b").set(5);
      assert.sameOrderedMembers(data.valueStore.dataSend.get("b") || [], [5]);
    });
    it("sets value when array of number is passed", function () {
      value(selfName, "b").set([2, 3, 4]);
      assert.sameOrderedMembers(
        data.valueStore.dataSend.get("b") || [],
        [2, 3, 4]
      );
    });
    it("sets value recursively when object is passed", function () {
      value(selfName, "b").set({
        a: 3,
        b: { c: 5 },
        v: [8, 9, 10],
      });
      assert.sameOrderedMembers(data.valueStore.dataSend.get("b.a") || [], [3]);
      assert.sameOrderedMembers(data.valueStore.dataSend.get("b.b.c") || [], [
        5,
      ]);
      assert.sameOrderedMembers(
        data.valueStore.dataSend.get("b.v") || [],
        [8, 9, 10]
      );
    });
    it("triggers change event", function () {
      let called = 0;
      value(selfName, "b").addListener((v) => {
        ++called;
        assert.strictEqual(v.member.name, selfName);
        assert.strictEqual(v.name, "b");
      });
      value(selfName, "b").set(1);
      assert.strictEqual(called, 1);
    });
    it("throws error when member is not self", function () {
      assert.throws(() => value("a", "b").set(1), Error);
    });
  });
  describe("#time()", function () {
    it("returns time set in data.syncTimeStore", function () {
      data.syncTimeStore.setRecv("a", new Date(10000));
      assert.strictEqual(value("a", "b").time().getTime(), 10000);
    });
  });
});

describe("Text Tests", function () {
  const selfName = "test";
  let data: ClientData;
  const text = (member: string, field: string) =>
    new Text(new Field(data, member, field));
  beforeEach(function () {
    data = new ClientData(selfName, () => undefined);
  });
  describe("#member", function () {
    it("returns Member object with its member name", function () {
      assert.instanceOf(text("a", "b").member, Member);
      assert.strictEqual(text("a", "b").member.name, "a");
    });
  });
  describe("#name", function () {
    it("returns field name", function () {
      assert.strictEqual(text("a", "b").name, "b");
    });
  });
  describe("#child()", function () {
    it("returns child Value object", function () {
      const c = text("a", "b").child("c");
      assert.instanceOf(c, Text);
      assert.strictEqual(c.member.name, "a");
      assert.strictEqual(c.name, "b.c");
    });
  });
  describe("#tryGet()", function () {
    it("returns null by default", function () {
      assert.isNull(text("a", "b").tryGet());
    });
    it("returns value if data.textStore.dataRecv is set", function () {
      data.textStore.dataRecv.set("a", new Map([["b", "aaa"]]));
      assert.strictEqual(text("a", "b").tryGet(), "aaa");
    });
    it("sets request when member is not self name", function () {
      text("a", "b").tryGet();
      assert.strictEqual(data.textStore.req.get("a")?.get("b"), 1);
    });
    it("does not set request when member is self name", function () {
      text(selfName, "b").tryGet();
      assert.isEmpty(data.textStore.req);
    });
  });
  describe("#get()", function () {
    it("returns empty string by default", function () {
      assert.strictEqual(text("a", "b").get(), "");
    });
    it("returns value if data.textStore.dataRecv is set", function () {
      data.textStore.dataRecv.set("a", new Map([["b", "aaa"]]));
      assert.strictEqual(text("a", "b").get(), "aaa");
    });
    it("sets request when member is not self name", function () {
      text("a", "b").get();
      assert.strictEqual(data.textStore.req.get("a")?.get("b"), 1);
    });
    it("does not set request when member is self name", function () {
      text(selfName, "b").get();
      assert.isEmpty(data.textStore.req);
    });
  });
  describe("#set()", function () {
    it("sets value when string is passed", function () {
      text(selfName, "b").set("aaa");
      assert.strictEqual(data.textStore.dataSend.get("b"), "aaa");
    });
    it("sets value recursively when object is passed", function () {
      text(selfName, "b").set({
        a: "a",
        b: { c: "bc" },
      });
      assert.strictEqual(data.textStore.dataSend.get("b.a"), "a");
      assert.strictEqual(data.textStore.dataSend.get("b.b.c"), "bc");
    });
    it("triggers change event", function () {
      let called = 0;
      text(selfName, "b").addListener((t) => {
        ++called;
        assert.strictEqual(t.member.name, selfName);
        assert.strictEqual(t.name, "b");
      });
      text(selfName, "b").set("aaa");
      assert.strictEqual(called, 1);
    });
    it("throws error when member is not self", function () {
      assert.throws(() => text("a", "b").set("a"), Error);
    });
  });
  describe("#time()", function () {
    it("returns time set in data.syncTimeStore", function () {
      data.syncTimeStore.setRecv("a", new Date(10000));
      assert.strictEqual(text("a", "b").time().getTime(), 10000);
    });
  });
});
describe("Log Tests", function () {
  const selfName = "test";
  let data: ClientData;
  const log = (member: string) => new Log(new Field(data, member, ""));
  beforeEach(function () {
    data = new ClientData(selfName, () => undefined);
  });
  describe("#member", function () {
    it("returns Member object with its member name", function () {
      assert.instanceOf(log("a").member, Member);
      assert.strictEqual(log("a").member.name, "a");
    });
  });
  describe("#tryGet()", function () {
    it("returns null by default", function () {
      assert.isNull(log("a").tryGet());
    });
    it("returns value if data.logStore.dataRecv is set", function () {
      data.logStore.dataRecv.set("a", [
        { level: 1, time: new Date(), message: "a" },
      ]);
      assert.lengthOf(log("a").tryGet() || [], 1);
      assert.strictEqual((log("a").tryGet() || [])[0]?.level, 1);
      assert.strictEqual((log("a").tryGet() || [])[0]?.message, "a");
    });
    it("sets request when member is not self name", function () {
      log("a").tryGet();
      assert.strictEqual(data.logStore.req.get("a"), true);
    });
    it("does not set request when member is self name", function () {
      log(selfName).tryGet();
      assert.isEmpty(data.logStore.req);
    });
  });
});
