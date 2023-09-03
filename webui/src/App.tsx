import { useState, useEffect, useRef } from "react";
import { Client, Member } from "webcface";
import "./index.css";
import { LayoutMain } from "./components/layout";
import { Header } from "./components/header";
import { SideMenu } from "./components/sideMenu";
import {
  MemberValues,
  MemberTexts,
  MemberFuncs,
  MemberLogs,
} from "./libs/stateTypes";

export default function App() {
  const client = useRef<Client | null>(null);
  useEffect(() => {
    client.current = new Client("a");
  }, []);
  useEffect(() => {
    const i = setInterval(() => {
      client.current?.sync();
    }, 100);
    return () => clearInterval(i);
  }, [client]);

  const [menuOpen, setMenuOpen] = useState<boolean>(false);
  const [openedCards, setOpenedCards] = useState<string[]>([]);
  const isOpened = (key: string) => openedCards.includes(key);
  const openedOrder = (key: string) => openedCards.indexOf(key) || 0;
  const toggleOpened = (key: string) => {
    if (openedCards.includes(key)) {
      setOpenedCards(openedCards.filter((n) => n !== key));
    } else {
      setOpenedCards(openedCards.concat([key]));
    }
  };
  const moveOrder = (key: string) => {
    setOpenedCards(openedCards.filter((n) => n !== key).concat([key]));
  };

  return (
    <div className="min-h-screen h-max bg-neutral-100">
      <nav className="bg-green-300 w-full h-12 px-2 drop-shadow-lg">
        <Header menuOpen={menuOpen} setMenuOpen={setMenuOpen} />
      </nav>
      <nav
        className={
          "absolute top-10 right-2 w-72 h-max max-h-[75%] p-2 " +
          "rounded-lg shadow-lg overflow-x-hidden overflow-y-auto bg-white " +
          "transition duration-100 origin-top-right " +
          (menuOpen
            ? "ease-out opacity-100 scale-100 z-[1000] "
            : "ease-in opacity-0 scale-90 -z-10 ")
        }
      >
        <SideMenu
          client={client}
          isOpened={isOpened}
          toggleOpened={toggleOpened}
        />
      </nav>
      <main className="p-2">
        <LayoutMain
          client={client}
          isOpened={isOpened}
          openedOrder={openedOrder}
          moveOrder={moveOrder}
        />
      </main>
    </div>
  );
}
