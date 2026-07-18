#include <sendafrica/sendafrica.hpp>

#include <iostream>

int main() {
    // Reads SENDAFRICA_API_KEY from the environment.
    sendafrica::ClientOptions opts;
    opts.debug = true;

    try {
        sendafrica::Client client(std::nullopt, opts);

        auto analysis = client.sms().analyze("Habari, karibu SendAfrica!");
        std::cout << "encoding=" << analysis.encoding << " parts=" << analysis.parts
                  << " credits=" << analysis.credits << "\n";

        auto result = client.sms().send("0712345678", "Welcome to SendAfrica");
        std::cout << "message_id=" << result.message_id << " status=" << result.status << "\n";
    } catch (const sendafrica::Error& e) {
        std::cerr << "SendAfrica error: " << e.message() << "\n";
        return 1;
    }

    return 0;
}
