#include <stdlib.h>
#include <stdio.h>
#include "../../PO8e.h"
#include "../../compat.h"
#include "../../support.h"

int main(int argc, char **argv)
{
    HIGH_RES_TIMER timer;
    int count = 0, total;
    long long totalBytes;
    PO8e *cards[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

    total = PO8e::cardCount();
    printf("Found %d card(s) in the system.\n", total);
    if (0 == total)
    {
        printf("  exiting\n");
        exit(1);
    }

    for(int x = 0; x < total; x++)
    {
        printf(" Connecting to card %d\n", x);
        cards[count] = PO8e::connectToCard(x);
        if (cards[count] == NULL)
            printf("  connection failed\n");
        else
        {
            printf("  established connection %p\n", (void*)cards[count]);
//TODO: have to expose the card ports MUCH better
            if (! cards[count]->startCollecting())
            {
                printf("  startCollecting() failed with: %d\n",
                       cards[count]->getLastError());
                PO8e::releaseCard(cards[count]);
            }
            else
            {
                printf("  card is collecting incoming data.\n");
                count++;
            }
        }
    }

    // wait for streaming to start on the first card
    printf("Waiting for the stream to start on card 0\n");
    while(cards[0]->samplesReady() == 0)
        usleep(5000);

    // start the timer used to compute the speed and set the collected bytes to 0
    timer.start();
    totalBytes = 0;

    int stoppedCount = 0, pos = 0;
    while(stoppedCount < count)
    {
        // compute the rate in megabytes per second
        double rate = totalBytes / timer.elapsedSeconds() / (1024 * 1024);
        char posChar = "|/-\\"[pos];
        pos = (pos + 1) % 4;

        // if we are working with just one card, work with it much more efficiently
        if (count == 1 &&
            ! cards[0]->waitForDataReady())
            break;

        int waitCount = 0;
        stoppedCount = 0;
        for(int x = 0; x < count; x++)
        {
            bool stopped = false;
            size_t numSamples = cards[x]->samplesReady(&stopped);
            if (stopped)
                stoppedCount++;
            else if (numSamples > 0)
            {
                int channel = random() % cards[x]->numChannels();

                printf("Card %d has %4d samples of %4d channels.  Current rate: %.2lf MB/s %c \r", x, numSamples, cards[x]->numChannels(), rate, posChar);
                fflush(stdout);

                short bufferA[8192] = {42};
                if (cards[x]->readChannel(channel,
                                          bufferA, (int)numSamples) != numSamples)
                    printf("  reading %d samples from channel %d failed\n",
                           numSamples, channel);
                totalBytes += numSamples * cards[x]->numChannels() * cards[x]->dataSampleSize();

                short bufferB[8192];
                short temp[1024];
                for(int i = 0; i < (int)numSamples; i++)
                {
                    if (cards[x]->readBlock(temp, 1) != 1)
                        printf("  reading block of %d samples failed\n", numSamples);
                    bufferB[i] = temp[channel];
                    cards[x]->flushBufferedData(1);
                }

                // print a comparison of the two ways of reading data
                for(int y = 0; y < (int)numSamples; y++)
                {
                    short a, b;
                    a = bufferA[y];
                    b = bufferB[y];
                    if (a != b)
                        fprintf(stderr, "Sample mismatch on channel %d! %d\t!=\t%d\t\n", channel, a, b);
                }
            }
            else
                waitCount++;
        }
    }
    printf("\n");

    for(int x = 0; x < count; x++)
    {
        printf("Releasing card %d\n", x);
        PO8e::releaseCard(cards[x]);
    }

    return 0;
}
