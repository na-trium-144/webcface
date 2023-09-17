import { assert } from "chai";
import { Func, AnonymousFunc, FuncCallback } from "../src/func.js";
import {
  View,
  viewComponents,
  viewComponentTypes,
  viewColor,
} from "../src/view.js";
import { ClientData } from "../src/clientData.js";
import { Field, FieldBase } from "../src/field.js";
import { Member } from "../src/member.js";
import { valType } from "../src/message.js";

describe("View Tests", function () {
  const selfName = "test";
  let data: ClientData;
  const view = (member: string, field: string) =>
    new View(new Field(data, member, field));
  const func = (member: string, field: string) =>
    new Func(new Field(data, member, field));
  const afunc1 = (func: FuncCallback) =>
    new AnonymousFunc(new Field(data, selfName, ""), func, valType.none_, []);
  beforeEach(function () {
    data = new ClientData(selfName, () => undefined);
  });
  describe("#member", function () {
    it("returns Member object with its member name", function () {
      assert.instanceOf(view("a", "b").member, Member);
      assert.strictEqual(view("a", "b").member.name, "a");
    });
  });
  describe("#name", function () {
    it("returns field name", function () {
      assert.strictEqual(view("a", "b").name, "b");
    });
  });
  describe("#set", function () {
    it("set components as Message.ViewComponent object", function () {
      view(selfName, "a").set([
        "a\n",
        1,
        viewComponents.text("aaa", {
          textColor: viewColor.yellow,
          bgColor: viewColor.green,
        }),
        viewComponents.newLine(),
        viewComponents.button("f", func(selfName, "f")),
        viewComponents.button(
          "a",
          afunc1(() => undefined)
        ),
        viewComponents.button("a2", () => undefined),
      ]);
      const v = data.viewStore.dataRecv.get(selfName)?.get("a") || [];
      assert.include(v[0], { t: viewComponentTypes.text, x: "a" });
      assert.include(v[1], { t: viewComponentTypes.newLine });
      assert.include(v[2], { t: viewComponentTypes.text, x: "1" });
      assert.include(v[3], {
        t: viewComponentTypes.text,
        x: "aaa",
        c: viewColor.yellow,
        b: viewColor.green,
      });
      assert.include(v[4], { t: viewComponentTypes.newLine });
      assert.include(v[5], {
        t: viewComponentTypes.button,
        x: "f",
        L: selfName,
        l: "f",
      });
      assert.include(v[6], {
        t: viewComponentTypes.button,
        x: "a",
        L: selfName,
      });
      assert.isNotEmpty(v[6].l);
      assert.include(v[7], {
        t: viewComponentTypes.button,
        x: "a2",
        L: selfName,
      });
      assert.isNotEmpty(v[7].l);
    });
    it("triggers change event", function () {
      let called = 0;
      view(selfName, "b").addListener((v) => {
        ++called;
        assert.strictEqual(v.member.name, selfName);
        assert.strictEqual(v.name, "b");
      });
      view(selfName, "b").set([]);
      assert.strictEqual(called, 1);
    });
    it("throws error when member is not self", function () {
      assert.throws(() => view("a", "b").set([]), Error);
    });
  });
});
