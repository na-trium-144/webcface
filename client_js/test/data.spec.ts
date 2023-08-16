import { assert } from "chai";
import { Client } from "../src/client.js";
import { FieldBase } from "../src/clientData.js";
import { Member, Value, Text, Func } from "../src/data.js";
import { argType } from "../src/message.js";

describe("Data tests", function () {
  const FieldBaseEqual = (f1: FieldBase, f2: FieldBase) => {
    assert.strictEqual(f1.data, f2.data);
    assert.strictEqual(f1.member_, f2.member_);
    assert.strictEqual(f1.field_, f2.field_);
  };
  describe("Member", function () {
    let cli1: Client;
    beforeEach(function () {
      cli1 = new Client("test1");
    });
    afterEach(function () {
      cli1.close();
    });
    it("Client is Member", function () {
      assert.strictEqual(cli1.member_, "test1");
      assert.strictEqual(cli1.member_, cli1.member("test1").member_);
      assert.instanceOf(cli1, Member);
    });
    it("#name is member name", function () {
      assert.strictEqual(cli1.name, cli1.member_);
    });
    it("#value returns correct Value object", function () {
      const v = cli1.value("a");
      FieldBaseEqual(v, new FieldBase(cli1.data, cli1.name, "a"));
      assert.instanceOf(v, Value);
    });
    it("#text returns correct Text object", function () {
      const v = cli1.text("a");
      FieldBaseEqual(v, new FieldBase(cli1.data, cli1.name, "a"));
      assert.instanceOf(v, Text);
    });
    it("#func returns correct Func object", function () {
      const v = cli1.func("a");
      FieldBaseEqual(v, new FieldBase(cli1.data, cli1.name, "a"));
      assert.instanceOf(v, Func);
    });
    it("#values returns array of Value objects", function () {
      assert.isEmpty(cli1.values());
      cli1.value("a").set(1);
      assert.lengthOf(cli1.values(), 1);
      FieldBaseEqual(cli1.values()[0], cli1.value("a"));
    });
    it("#texts returns array of Text objects", function () {
      assert.isEmpty(cli1.texts());
      cli1.text("a").set("aaa");
      assert.lengthOf(cli1.texts(), 1);
      FieldBaseEqual(cli1.texts()[0], cli1.text("a"));
    });
    it("#funcs returns array of Func objects", function () {
      assert.isEmpty(cli1.funcs());
      cli1.func("a").set(() => undefined);
      assert.lengthOf(cli1.funcs(), 1);
      FieldBaseEqual(cli1.funcs()[0], cli1.func("a"));
    });
  });
  describe("Value", function () {
    let cli1: Client;
    let v: Value;
    beforeEach(function () {
      cli1 = new Client("test1");
      v = cli1.value("a");
      v.set(1);
    });
    afterEach(function () {
      cli1.close();
    });
    it("#name is field name", function () {
      assert.strictEqual(v.name, v.field_);
      assert.strictEqual(v.name, "a");
    });
    it("#member returns Member", function () {
      assert.strictEqual(v.member.name, cli1.name);
      assert.instanceOf(v.member, Member);
    });
    it("#tryGet returns value or null", function () {
      assert.strictEqual(v.tryGet(), 1);
      assert.strictEqual(cli1.value("b").tryGet(), null);
    });
    it("#get returns value or 0", function () {
      assert.strictEqual(v.get(), 1);
      assert.strictEqual(cli1.value("b").get(), 0);
    });
  });
  describe("Text", function () {
    let cli1: Client;
    let v: Text;
    beforeEach(function () {
      cli1 = new Client("test1");
      v = cli1.text("a");
      v.set("aaa");
    });
    afterEach(function () {
      cli1.close();
    });
    it("#name is field name", function () {
      assert.strictEqual(v.name, v.field_);
      assert.strictEqual(v.name, "a");
    });
    it("#member returns Member", function () {
      assert.strictEqual(v.member.name, cli1.name);
      assert.instanceOf(v.member, Member);
    });
    it("#tryGet returns value or null", function () {
      assert.strictEqual(v.tryGet(), "aaa");
      assert.strictEqual(cli1.text("b").tryGet(), null);
    });
    it("#get returns value or empty string", function () {
      assert.strictEqual(v.get(), "aaa");
      assert.strictEqual(cli1.text("b").get(), "");
    });
  });
  describe("Func", function () {
    let cli1: Client;
    let v: Func;
    let callCount: number;
    let passedArg: any[];
    beforeEach(function () {
      cli1 = new Client("test1");
      v = cli1.func("a");
      v.set(() => {
        callCount++;
      });
      cli1.func("b").set(() => {
        return 123;
      });
      cli1.func("c").set(
        (a: number, b: boolean, c: string) => {
          passedArg = [a, b, c];
        },
        argType.none_,
        [
          { name: "a", type: argType.number_ },
          { name: "b", type: argType.boolean_ },
          { name: "c", type: argType.string_ },
        ]
      );
      callCount = 0;
      passedArg = [];
    });
    afterEach(function () {
      cli1.close();
    });
    it("#name is field name", function () {
      assert.strictEqual(v.name, v.field_);
      assert.strictEqual(v.name, "a");
    });
    it("#member returns Member", function () {
      assert.strictEqual(v.member.name, cli1.name);
      assert.instanceOf(v.member, Member);
    });
    describe("#runAsync", function () {
      it("runs function", async function () {
        await v.runAsync().result;
        assert.strictEqual(callCount, 1);
      });
      it("returns value", async function () {
        assert.strictEqual(await cli1.func("b").runAsync().result, 123);
      });
      it("converts and pass arguments", async function () {
        await cli1.func("c").runAsync("1", "2", "3").result;
        assert.sameOrderedMembers(passedArg, [1, true, "3"]);
      });
    });
  });
});
