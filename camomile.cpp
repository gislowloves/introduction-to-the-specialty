#include <exception>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <sstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class IEventik {
public:
    virtual std::string show() = 0;
    virtual std::string applyChoice(int choice) = 0;
    virtual ~IEventik() = default;
};

class Character {
private:
    std::string name;
    int stat;

public:
    Character(const std::string& name, int initialStat = 25)
        : name(name), stat(initialStat) {}

    int getStat() const {
        if (stat > 50) return 50;
        if (stat < 0) return 0;
        return stat;
    }

    void setStat(int value) {
        stat = value;
    }

    void operator()(int delta) {
        stat += delta;
    }
};

class Village {
private:
    int mood;

public:
    Village(int initialMood = 50) : mood(initialMood) {}

    void operator()(int delta) {
        mood += delta;
    }

    void setMood(int value) {
        mood = value;
    }

    int getMood() const {
        if (mood > 100) return 100;
        if (mood < 0) return 0;
        return mood;
    }
};

class ChoiceEvent : public IEventik {
protected:
    struct Option {
        std::string varText;
        std::string resultText;
        std::map<std::string, int> statChanges;
        int moodChange;
    };

    std::string title;
    std::vector<Option> options;
    std::map<std::string, std::shared_ptr<Character>> characters;
    Village& village;

public:
    ChoiceEvent(std::string title,
                std::map<std::string, std::shared_ptr<Character>>& characters,
                Village& village)
        : title(title), characters(characters), village(village) {}

    std::string show() override {
        std::stringstream ss;
        ss << title << "\n\n";

        for (int i = 0; i < 3; i++) {
            ss << i + 1 << ". " << options[i].varText << "\n";
        }

        return ss.str();
    }

    std::string applyChoice(int choice) override {
        if (choice < 1 || choice > 3) {
            return json{{"error", "invalid choice"}}.dump();
        }

        Option opt = options[choice - 1];

        for (auto& [name, delta] : opt.statChanges) {
            (*characters[name])(delta);
        }

        village(opt.moodChange);

        json j;
        j["result"] = opt.resultText;

        json statsJson = json::object();
        for (auto& [name, c] : characters) {
            statsJson[name] = c->getStat();
        }

        j["stats"] = statsJson;
        j["village"] = village.getMood();

        return j.dump();
    }
};

class ChinaDelegationEvent : public ChoiceEvent {
public:
    ChinaDelegationEvent(std::map<std::string, std::shared_ptr<Character>>& chars,Village& village) : ChoiceEvent(
        "Приехал Чунь Пунь Хрюк с китайской делегацией, они предлагают сотрудничество.\n"
        "С их стороны предоставление новых технологий для развития деревни,\n"
        "но жители деревни отдают под контроль часть Ромашковой долины.\n"
        "Какое решение принять? (¬‿¬ ) ", chars, village) 
        {options = {
            {"Согласиться на предложение и устроить в честь этого события большой пир.",
                "Чунь Пунь Хрюк был приятно удивлён такой гостеприимной встречей\n"
                "и сразу полюбил деревню смешариков.\n"
                "Было принято решение отдать все технологии даром,\n"
                "но Чунь Пунь Хрюк иногда будет навещать Ромашковую Долину.\n\n"
                "Кар-Карыч смог повысить свои навыки дипломатии,\n"
                "общаясь с иностранными гостями, а Пин был невероятно доволен\n"
                "новыми возможностями.\n\n"
                "Однако Копатычу пришлось потратить немало продовольствия,\n"
                "чтобы прокормить всех этих китайцев.",
                {{"Kar-Karych", +5}, {"Pin", +5}, {"Kopatych", -5}}, +15 },

                {"Согласиться на предложение и провести экскурсию по Ромашковой Долине.\n"
                "Может им понравятся наши окрестности, и они предложат что-то большее.",
                "Китайцам и правда понравилась деревня смешариков, но они остались голодными.\n"
                "Китайцы были готовы заключить контракт. Под их контроль ушла половина долины.\n"
                "Зато Кар-Карыч смог повысить свои навыки дипломатии,\n"
                "а Пин получил новые возможности в разработках.",
                {{"Kar-Karych", +5}, {"Pin", +5}},-50},

                {"Зачем нам эти китайцы, мы сами лучше всё сделаем!",
                "Ничего особо не поменялось, Кар-Карыч не смог прокачать свои навыки дипломатии,\n"
                "а Нюша осталась без китайского принца, зато Ёжик остался спокоен и непоколебим.",
                {{"Kar-Karych", -5}, {"Yozhik", +5}}, 0
            }
        };
    }
};

class OlympicsEvent : public ChoiceEvent {
public:
    OlympicsEvent(std::map<std::string, std::shared_ptr<Character>>& chars, Village& village) : ChoiceEvent(
        "Ромашковая Долина выбрана местом проведения следующих Олимпийских игр.\n"
        "Совунья и Крош пришли за советом по организации. (/ω＼)", chars, village) 
        {options = {
            {"Доверим организацию Крошу и Совунье. Они главные знатоки спорта.",
                "Крош и Совунья спортивные личности, технически место было оборудовано для спорта, были построены стадионы и турники.\n"
                "Олимпийские игры были проведены, но не вошли в историю. Нюше не хватило стразиков,\n"
                "а Ёжик переживал из-за большого наплыва иностранных смешариков (Они все пытались захапать его кактусы как сувениры).",
                {{"Krosh", +5}, {"Sovunya", +5}, {"Nyusha", -5}, {"Yozhik", -5}},
                +10},
            {"Передадим организацию Нюше и Барашу. Они смогут сделать всё красиво и привлечь много гостей.",
                "Они сделали всё красиво: вся Ромашковая Долина была в стразах от Нюши, а Бараш всем гостям рассказывал свою новую поэму, но спортивного инвентаря не хватало.\n"
                "Ёжик переживал из-за большого наплыва иностранных смешариков (Они все пытались захапать его кактусы как сувениры).",
                {{"Barash", +5}, {"Nyusha", +5}, {"Krosh", -5}, {"Sovunya", -5}, {"Yozhik", -5}},
                +10},
            {"Передадим организацию Пину и Лосяшу. Они придумали новую технологию. Эти Игры уж точно войдут в историю.",
                "Ничего не вышло. Всё развалилось. Технология вышла из-под контроля. Олимпийские Игры были отменены.\n"
                "Зато Ёжик не переживал из-за большого наплыва иностранных смешариков.\n"
                "Лосяш и Пин смогли попрактиковать свои навыки.",
                {{"Losyash", +5}, {"Pin", +5}, {"Yozhik", +5},
                 {"Barash", -5}, {"Nyusha", -5}, {"Krosh", -15}, {"Sovunya", -5}},
                -10
            }
        };
    }
};

class NyushaBirthdayEvent : public ChoiceEvent {
public:
    NyushaBirthdayEvent(std::map<std::string, std::shared_ptr<Character>>& chars, Village& village) : ChoiceEvent(
        "У Нюши день рождения. Жители Ромашковой Долины думают, как поздравить её? (*˘︶˘*).｡.:*♡ ", chars, village) 
        {options = {
            {"Пин предлагает снова создать шоколадную машину, но по новым технологиям.",
                "Вы не помните, что произошло в 43 серии 2 сезона «Сладкая жизнь»?\n"
                "Снова произошло шоколадное извержение. Пин смог попрактиковать свои навыки.\n"
                "Копатыч потерял весь свой урожай. Ёжик сильно перепугался. Нюша осталась довольной.\n"
                "Вся Долина затоплена шоколадом.",
                {{"Nyusha", +10}, {"Pin", +5}, {"Yozhik", -5}, {"Kopatych", -5}},
                -15},
            {"Бараш предлагает организовать концерт в честь Нюши.",
                "Концерт получился на славу. Бараш перечитал все свои поэмы для Нюши.\n"
                "Нюша была довольна огромным количеством комплиментов. Концерт прошёл спокойно и приятно.",
                {{"Barash", +10}, {"Nyusha", +5}, {"Yozhik", +5}},
                +15},
            {"Совунья предлагает провести праздничный ужин.",
                "Ужин прошёл вкусно и спокойно. Были потрачены запасы Копатыча. Крошу было чуть-чуть скучно.",
                {{"Nyusha", +5}, {"Sovunya", +5}, {"Yozhik", +5}, {"Kopatych", -5}, {"Krosh", -5}},
                +10
            }
        };
    }
};

class AlleyEvent : public ChoiceEvent {
public:
    AlleyEvent(std::map<std::string, std::shared_ptr<Character>>& chars, Village& village) : ChoiceEvent(
        "Копатыч решил посадить деревья и сделать аллею, но остальные смешарики тоже хотят внести свой вклад. Что в итоге сделать? Σ(￣。￣ﾉ) ", chars, village) 
        {options = {
            {"Посадить только деревья, как изначально хотел Копатыч. Деревья лишними не будут.",
                "Получилось хорошо, но взрослому поколению негде сидеть, они быстро устают.\n"
                "Настроение не поменялось (Ну был лес и лес, ничего не поменялось).",
                {{"Yozhik", +10}, {"Kopatych", +5}, {"Sovunya", -5}, {"Kar-Karych", -5}},
                0},
            {"Посадить деревья, добавить лавочки. Не так экологично, но удобно.",
                "Всем смешарикам очень понравилось, но Крошу захотелось ещё добавить туда турники.\n"
                "Нюше понравилась эта романтическая атмосфера.",
                {{"Yozhik", +10}, {"Kopatych", +5}, {"Sovunya", +5}, {"Kar-Karych", +5}, {"Krosh", -5}},
                +5},
            {"Все смешарики начали толпиться, каждый хотел внести свой вклад, царила неразбериха. Но может получится что-то хорошее?",
                "Получилась аллея имени меня (2 сезон 83 серия «Место в истории»).\n"
                "Все оставили свой след в вечности.",
                {{"Krosh", +5}, {"Yozhik", +5}, {"Nyusha", +5}, {"Barash", +5}, 
                 {"Losyash", +5}, {"Pin", +5}, {"Sovunya", +5}, {"Kar-Karych", +5}, {"Kopatych", +5}},
                +15
            }
        };
    }
};

class DiseaseEvent : public ChoiceEvent {
public:
    DiseaseEvent(std::map<std::string, std::shared_ptr<Character>>& chars, Village& village) : ChoiceEvent(
        "Ромашковую Долину сразила некая болезнь, Совунья сильно обеспокоена.\n"
        "Лосяш предлагает протестировать его новую микстуру, но Совунья насторожена. (ﾉ_ヽ) ", chars, village) 
        {options = {
            {"Прислушаться к Совунье и применить только народные методы",
                "Как говорится, народные методы победят любую болезнь.\n"
                "Ну и здесь они сработали, правда медленно. Ёжик очень боялся использовать непроверенную микстуру.\n"
                "Плюсом у Нюши кожа очистилась, прямо сияет.",
                {{"Sovunya", +10}, {"Nyusha", +5}, {"Yozhik", +5}, {"Losyash", -10}},
                +10},
            {"Использовать микстуру, но одновременно с ней применить народные методы Совуньи.\n"
                "Если микстура не сработает, то народные методы уж точно сработают.",
                "Лосяш настолько обрадовался применению микстуры, что не подумал о возможной несовместимости с другими лекарствами.\n"
                "Долина на грани вымирания.",
                {{"Losyash", +5}, {"Krosh", -5}, {"Yozhik", -5}, {"Nyusha", -5}, {"Barash", -5},
                 {"Pin", -5}, {"Sovunya", -5}, {"Kar-Karych", -5}, {"Kopatych", -5}},
                -45},
            {"Использовать только новомодную микстуру. Лосяш умный, не подведёт.",
                "Микстура работает, смешарики быстро победили болезнь, но у лекарства много побочных эффектов…\n"
                "У Нюши, например, кожа высохла.",
                {{"Losyash", +10}, {"Sovunya", -10}, {"Nyusha", -5}},
                +1
            }
        };
    }
};

class KopatychChoiceEvent : public ChoiceEvent {
public:
    KopatychChoiceEvent(std::map<std::string, std::shared_ptr<Character>>& chars, Village& village) : ChoiceEvent(
        "К Копатычу приезжает его давний друг из мегаполиса,\n"
        "он предлагает ему вернуться в город и продолжить кинокарьеру.\n"
        "Друзья любят Копатыча и сомневаются, отпускать его или нет. Что же делать? (・・;)ゞ ", chars, village) 
        {options = {
            {"Отпустить Копатыча, ведь это его дело жизни (вспомните фильм «Смешарики. Начало»)",
                "Как Вы могли отпустить Копатыча?(((\n"
                "Он покидает деревню на поиски своей судьбы в мегаполисе.",
                {{"Kopatych", -5}, {"Krosh", -5}, {"Yozhik", -5}, {"Nyusha", -5},
                 {"Barash", -5}, {"Losyash", -5}, {"Pin", -5}, {"Sovunya", -5}, {"Kar-Karych", -5}},
                -30},
            {"Не отпускать, мы же его любим.",
                "Копатыч ждал именно этого ответа. Он остаётся в деревне и продолжает заботиться о всех.",
                {{"Kopatych", +5}, {"Krosh", +5}, {"Yozhik", +5}, {"Nyusha", +5},
                 {"Barash", +5}, {"Losyash", +5}, {"Pin", +5}, {"Sovunya", +5}, {"Kar-Karych", +5}},
                +20},
            {"Съездить в город всем вместе на экскурсию.",
                "Копатыча увлекли городские жители, он остался и продолжил карьеру.\n"
                "Вся деревня возвращается домой с новым опытом, но часть продовольствия потеряна.",
                {{"Kopatych", -100}, {"Krosh", -5}, {"Yozhik", -5}, {"Nyusha", -5},
                 {"Barash", -5}, {"Losyash", -5}, {"Pin", -5}, {"Sovunya", -5}, {"Kar-Karych", -5}},
                -25
            }
        };
    }
};

class BarashInspirationEvent : public ChoiceEvent {
public:
    BarashInspirationEvent(std::map<std::string, std::shared_ptr<Character>>& chars, Village& village) : ChoiceEvent(
        "Бараш долго сидел и пытался написать хорошие стихи, но у него пропало вдохновение.\n"
        "Все думают, как ему помочь. Что Вы предложите? ╮(￣～￣)╭ ", chars, village) 
        {options = {
            {"Нюша всегда мечтала быть самой красивой, можно предложить ей стать его музой.",
                "Нюша попозировала для Бараша, он вдохновился её элегантностью и женственностью\n"
                "и написал целый ряд сочинений. Жизнь в долине не сильно изменилась.",
                {{"Nyusha", +5}, {"Barash", +5}},
                0},
            {"Крош и Ёжик наплевательски к поэзии предложили поиграть в футбол.\n"
                "Может и правда стоит отвлечься? Вдохновение не надо искать, оно само приходит.",
                "Бараш был возмущён таким предложением, ему сейчас явно не до какого-то там футбола!\n"
                "Но Крош и Ёжик отлично поиграли и без Бараша. Нюша расстроилась, что не смогла попозировать.",
                {{"Barash", -5}, {"Nyusha", -5}, {"Krosh", +5}, {"Yozhik", +5}},
                0},
            {"Поэт — личность тонкая, лучше его пока не трогать, а то мало ли что…",
                "Бараш ушёл в горы в поисках вдохновения, нашёл там небольшую хижину, и там ему стало действительно хорошо.\n"
                "Оказывается, уединение это именно то, что было ему нужно в данный момент.\n"
                "Кто поймёт этих поэтов?) Вернувшись к остальным смешарикам он наконец рассказал всем свои творения:\n"
                "«Писать стихи — моя стихия\n"
                "И днём, и ночью пишу стихи я»\n"
                "Всем очень понравилось.",
                {{"Barash", +10}},
                +10
            }
        };
    }
};

std::map<std::string, std::shared_ptr<Character>> makeCharacters(const json& state) {
    const std::vector<std::pair<std::string, std::string>> defaults = {
        {"Krosh", "Крош"},
        {"Yozhik", "Ёжик"},
        {"Nyusha", "Нюша"},
        {"Barash", "Бараш"},
        {"Losyash", "Лосяш"},
        {"Pin", "Пин"},
        {"Sovunya", "Совунья"},
        {"Kar-Karych", "Кар-Карыч"},
        {"Kopatych", "Копатыч"}
    };

    json stats = json::object();
    if (state.contains("stats") && state["stats"].is_object()) {
        stats = state["stats"];
    }

    std::map<std::string, std::shared_ptr<Character>> characters;

    for (const auto& [key, displayName] : defaults) {
        int initialStat = 25;
        if (stats.contains(key) && stats[key].is_number_integer()) {
            initialStat = stats[key].get<int>();
        }

        characters[key] = std::make_shared<Character>(displayName, initialStat);
    }

    return characters;
}

json makeState(const std::map<std::string, std::shared_ptr<Character>>& characters,
               const Village& village,
               int day) {
    json statsJson = json::object();
    for (const auto& [name, character] : characters) {
        statsJson[name] = character->getStat();
    }

    return {
        {"day", day},
        {"stats", statsJson},
        {"village", village.getMood()}
    };
}

int main() {
    json payload;

    try {
        std::string input;
        std::getline(std::cin, input, '\0');

        if (input.empty()) {
            std::cout << json{{"error", "empty payload"}}.dump();
            return 0;
        }

        payload = json::parse(input);
    } catch (const std::exception&) {
        std::cout << json{{"error", "invalid json payload"}}.dump();
        return 0;
    }

    if (!payload.contains("mode") || !payload["mode"].is_string()) {
        std::cout << json{{"error", "mode is required"}}.dump();
        return 0;
    }

    if (!payload.contains("day") || !payload["day"].is_number_integer()) {
        std::cout << json{{"error", "day is required"}}.dump();
        return 0;
    }

    json state = json::object();
    if (payload.contains("state") && payload["state"].is_object()) {
        state = payload["state"];
    }

    std::map<std::string, std::shared_ptr<Character>> characters = makeCharacters(state);

    int initialVillageMood = 50;
    if (state.contains("village") && state["village"].is_number_integer()) {
        initialVillageMood = state["village"].get<int>();
    }

    Village village(initialVillageMood);

    std::vector<std::shared_ptr<IEventik>> events = {
        std::make_shared<NyushaBirthdayEvent>(characters, village),
        std::make_shared<OlympicsEvent>(characters, village),
        std::make_shared<ChinaDelegationEvent>(characters, village),
        std::make_shared<AlleyEvent>(characters, village),
        std::make_shared<BarashInspirationEvent>(characters, village),
        std::make_shared<DiseaseEvent>(characters, village),
        std::make_shared<KopatychChoiceEvent>(characters, village)
    };

    int day = payload["day"].get<int>();
    if (day < 0 || day >= static_cast<int>(events.size())) {
        std::cout << json{{"error", "invalid day"}}.dump();
        return 0;
    }

    std::string mode = payload["mode"].get<std::string>();

    if (mode == "event") {
        std::cout << json{
            {"text", events[day]->show()},
            {"state", makeState(characters, village, day)}
        }.dump();
        return 0;
    }

    if (mode == "choice") {
        if (!payload.contains("choice") || !payload["choice"].is_number_integer()) {
            std::cout << json{{"error", "choice is required"}}.dump();
            return 0;
        }

        int choice = payload["choice"].get<int>();
        json result = json::parse(events[day]->applyChoice(choice));

        if (!result.contains("error")) {
            result["state"] = makeState(characters, village, day + 1);
        }

        std::cout << result.dump();
        return 0;
    }

    std::cout << json{{"error", "unknown mode"}}.dump();
    return 0;
}
